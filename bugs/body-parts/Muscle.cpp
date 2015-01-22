/*
 * Muscle.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 *
 *
 *  Muscle width determines its max effective torque
 *  Muscle length determines the insertion point which has an effect on the max torque and max speed of the joint.
 *  max muscle contraction (%) is constant -> longer muscle's length during a full contraction varies more.
 *  muscle contraction speed (m/s) is constant => a longer muscle (which needs to vary its length more) is slower.
 *  muscle length spectrum correlates 1:1 to the joint's rotation limits.
 *  	- minimum muscle length (max contraction) corresponds to the joint being at its closest limit
 *  	- max muscle length (min contraction) corresponds to the joint being wide open
 *  	=> this poses the problem to find the r (insertion point) that satisfies these conditions.
 *  the greater the ratio between muscle length variation and joint angle variation the greater the insertion point
 *  	(theoretical) of the muscle -> slower but more powerful coupling.
 *  muscle knows (by constructor) in which direction it actions the joint motor.
 *  muscle command signal is clamped to [0.0 : 1.0]
 *  joint motor speed is muscle's max angular speed
 *  joint max torque is signal.value * muscle's max torque
 *
 *  formulas for muscle:
 *
 *  l0 = muscle relaxed length												[m]
 *  l = muscle max contracted length < l									[m]
 *  contractionRatio = l/l0 < 1												[1]
 *  dx (length difference) = l0 - l = l0 * (1 - contractionRatio)			[m]
 *  h (distance from muscle's origin to joint)				 				[m]
 *  r (insertion distance from joint center)								[m]
 *  F (max muscle force) = constant * muscle.width							[N]
 *  tau (max torque) = F*sgn.val * r*sin(alpha) + F*sgn.val * h*sin(beta)	[Nm]
 *  r*sin(alpha) and h*sin(beta) are precomputed at commit time and stored into tables as functions of phi (joint angle) [1]
 */

#include "Muscle.h"
#include "Joint.h"
#include "Bone.h"
#include "BodyConst.h"
#include "../math/math2D.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/RenderContext.h"
#include "../neuralnet/InputSocket.h"
#include "../utils/UpdateList.h"
#include <glm/vec3.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <math.h>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(1.f,0.2f, 0.8f);

MuscleInitializationData::MuscleInitializationData()
	: aspectRatio(BodyConst::initialMuscleAspectRatio) {
	density.reset(BodyConst::MuscleDensity);
}

Muscle::Muscle(BodyPart* parent, Joint* joint, int motorDirSign)
	: BodyPart(parent, BODY_PART_MUSCLE, std::make_shared<MuscleInitializationData>())
	, muscleInitialData_(std::static_pointer_cast<MuscleInitializationData>(getInitializationData()))
	, inputSocket_(std::make_shared<InputSocket>(nullptr, 1.f))
	, joint_(joint)
	, rotationSign_(motorDirSign)
	, maxForce_(0)
	, maxJointAngularSpeed_(0)
	, phiToRSinAlphaHSinBeta_{0}
	, phiAngleStep_(0)
	, cachedPhiMin_(0)
#ifdef DEBUG_DRAW_MUSCLE
	, phiToDx_{0}
#endif
{
	// we need this for debug draw, since muscle doesn't create fixture, nor body
	keepInitializationData_ = true;
	dontCreateBody_ = true;

	std::shared_ptr<MuscleInitializationData> initData = muscleInitialData_.lock();
	registerAttribute(GENE_ATTRIB_ASPECT_RATIO, initData->aspectRatio);

	getUpdateList()->add(this);
}

Muscle::~Muscle() {
}

void Muscle::commit() {
	assert(joint_ != nullptr);

	std::shared_ptr<MuscleInitializationData> initData = muscleInitialData_.lock();

	// here we compute the characteristics of the muscle
	float w0 = sqrtf(initData->size / initData->aspectRatio); // relaxed width
	float l0 = initData->aspectRatio * w0; // relaxed length
	float dx = l0 * (1 - BodyConst::MuscleContractionRatio);

	maxForce_ = w0 * BodyConst::MuscleForcePerWidthRatio;

	/*
	 * h is the vector from muscle to joint
	 * r is the vector from the joint to insertion point
	 * t is the vector from muscle to insertion point
	 * alpha is the angle between r and t, the angle at which the muscle force is applied to the bone or gripper
	 * beta is the angle between h and t, the angle at which the muscle pulls from its parent (reaction force)
	 *
	 *
	 * equation to compute r:
	 * dx^4+4*h^2*r^2*p^2 - 4*dx^2*h*r*p - 4*dx^2*(h^2+r^2-2*h*r*CM); = 0
	 * delta = dx^2*(4*dx^2*h^2*CM^2 - 4*dx^2*h^2*p*CM + 4*h^4*p^2 - 4*dx^2*h^2 + dx^4)
	 *
	 *           dx^2*h*p - 2*dx^2*h*CM +/- sqrt(delta)
	 * r1,2 = -------------------------------------
	 *                 2*(h^2*p^2 - dx^2)
	 *
	 * where:
	 * p = CM-Cm
	 * CM = cos(phi_muscle - phi_MAX)
	 * Cm = cos(phi_muscle - phi_min)
	 *
	 * phi_muscle is the angle between -h and r0(phi0)
	 * phi_MAX is the maximum joint angle relative to phi0
	 * phi_min is the minimum joint angle relative to phi0
	 * phi0 is the angle insertion axis when the joint is at its 0 position.
	 * it is defined as the child-bone's local OX or OY axis (depending on the bone's aspect ratio),
	 * rotated into world space and translated into J (the joint position)
	 */

	// compute insertion axis (phi0):
	BodyPart* targetPart = joint_->getChild(0);
	bool useOY = false;
	if (targetPart->getType() == BODY_PART_BONE) {
		Bone* bone = dynamic_cast<Bone*>(targetPart);
		std::shared_ptr<BoneInitializationData> ptr = std::dynamic_pointer_cast<BoneInitializationData>(bone->getInitializationData());
		assert(ptr);
		if (ptr->aspectRatio < 1.f)
			useOY = true;
	}
	// this is the world angle of insertion axis in default joint position:
	float phi0 = targetPart->getWorldTransformation().z + (useOY ? PI/2 : 0);
	glm::vec2 phi0_v(glm::rotate(glm::vec2(1, 0), phi0));

	// compute the muscle angle (phi_muscle):
	glm::vec2 M(vec3xy(getWorldTransformation()));
	glm::vec2 J(vec3xy(joint_->getWorldTransformation()));
	glm::vec2 h_v(J-M);
	float h = glm::length(h_v);
	// at this time, both M, J, h and phi0_v are expressed in world space
	float phi_muscle = acosf(glm::dot(glm::normalize(-h_v), phi0_v));
	// phi_muscle is now relative to phi0

	// compute helper parameters:
	float phi_min = joint_->getLowerLimit();	// relative to phi0
	float phi_min_clamp = clamp(phi_min, phi_muscle - PI, 2*PI); // clamp to the angle which gives the greatest muscle length
	float phi_MAX = joint_->getUpperLimit();	// relative to phi0
	float phi_MAX_clamp = clamp(phi_MAX, -2*PI, phi_muscle); // clamp to the angle which gives the smallest muscle length
	if (rotationSign_ < 0)
		xchg(phi_min_clamp, phi_MAX_clamp);		// for negative rotation muscles, reverse the angles
	float Cm = cosf((phi_muscle - phi_min_clamp) * rotationSign_);
	float CM = cosf((phi_muscle - phi_MAX_clamp) * rotationSign_);
	float p = CM-Cm;

	cachedPhiMin_ = phi_min;

	float D = 4*sqr(h)*(sqr(dx)*(sqr(CM)-p*CM-1) + sqr(h*p)) + sqr(sqr(dx));
	assert(D>=0);
	float sqrtD = sqrt(D);
	float bneg = -2*sqr(dx)*h*CM + sqr(dx)*h*p;
	float denom = 1.f / (2*sqr(h*p) - 2*sqr(dx));
	float r1 = (bneg + dx*sqrtD) * denom;
	float r2 = (bneg - dx*sqrtD) * denom;
	float r = r1 < 0 ? r2 : (r2 < 0 ? r1 : min(r1, r2));	// use the smaller non-negative value

	// now compute the rsinalpha and hcosbeta tables for 10 intermediate steps:
	phiAngleStep_ = (phi_MAX - phi_min) / nAngleSteps;
	glm::vec2 h_v_norm(glm::normalize(h_v));
#ifdef DEBUG_DRAW_MUSCLE
	glm::vec2 t0_v(h_v + glm::rotate(phi0_v, phi_min_clamp) * r);	// longest tendon length
	float t0 = glm::length(t0_v);
#endif
	for (int i=0; i<nAngleSteps; i++) {
		float phi = phi_min + phiAngleStep_ * i;
		glm::vec2 r_v_norm(glm::rotate(phi0_v, phi));
		glm::vec2 t_v_norm(glm::normalize(h_v + r_v_norm*r));
		float alpha = acosf(glm::dot(t_v_norm, r_v_norm));
		float beta = acosf(glm::dot(h_v_norm, t_v_norm));
		phiToRSinAlphaHSinBeta_[i] = r * sinf(alpha) + h * sinf(beta);
#ifdef DEBUG_DRAW_MUSCLE
		phiToDx_[i] = t0 - glm::length(h_v + r_v_norm*r);
#endif
	}

	// must also compute max speed:
	maxJointAngularSpeed_ = joint_->getTotalRange() / dx * BodyConst::MuscleMaxLinearContractionSpeed;
}

glm::vec2 Muscle::getChildAttachmentPoint(float relativeAngle) const {
	std::shared_ptr<MuscleInitializationData> initData = muscleInitialData_.lock();
	// this also takes aspect ratio into account as if the angle is expressed
	// for an aspect ratio of 1:1, and then the resulting point is stretched along the edge.

	float hw = sqrtf(initData->size / initData->aspectRatio) * 0.5f; // half width
	float hl = initData->aspectRatio * hw; // half length
	// bring the angle between [-PI, +PI]
	relativeAngle = limitAngle(relativeAngle, 7*PI/4);
	if (relativeAngle < PI/4) {
		// front edge
		return glm::vec2(hl, sinf(relativeAngle) / sinf(PI/4) * hw);
	} else if (relativeAngle < 3*PI/4 || relativeAngle > 5*PI/4) {
		// left or right edge
		return glm::vec2(cosf(relativeAngle) / cosf(PI/4) * hl, relativeAngle < PI ? hw : -hw);
	} else {
		// back edge
		return glm::vec2(-hl, sinf(relativeAngle) / sinf(PI/4) * hw);
	}
}

void Muscle::draw(RenderContext const& ctx) {
	std::shared_ptr<MuscleInitializationData> initData = muscleInitialData_.lock();
	float aspectRatio = initData->aspectRatio;
#ifdef DEBUG_DRAW_MUSCLE
	if (committed_) {
		float w0 = sqrtf(initData->size / aspectRatio);
		float l0 = aspectRatio * w0;
		float dx = lerp_lookup(phiToDx_, nAngleSteps, getCurrentPhiSlice());
		aspectRatio *= sqr((l0 - dx) / l0);
	}
#endif
	float w = sqrtf(initData->size / aspectRatio);
	float l = aspectRatio * w;
	glm::vec3 worldTransform = getWorldTransformation();
	ctx.shape->drawRectangle(vec3xy(worldTransform), 0,
			glm::vec2(l, w), worldTransform.z, debug_color);
	ctx.shape->drawLine(
			vec3xy(worldTransform),
			vec3xy(worldTransform) + glm::rotate(getChildAttachmentPoint(0), worldTransform.z),
			0,
			debug_color);
}

float Muscle::getCurrentPhiSlice() {
	float angleSlice = (joint_->getJointAngle() - cachedPhiMin_) / phiAngleStep_;
	int iAngleSlice = (int) angleSlice;
	angleSlice -= iAngleSlice;
	if (iAngleSlice >= nAngleSteps)
		iAngleSlice = nAngleSteps - 1;
	if (iAngleSlice < 0)
		iAngleSlice = 0;
	return iAngleSlice + angleSlice;
}

void Muscle::update(float dt) {
	float signal_strength = clamp(inputSocket_->value, 0.f, 1.f);
	float RSinAlphaHSinBeta = lerp_lookup(phiToRSinAlphaHSinBeta_, nAngleSteps, getCurrentPhiSlice());
	float torque = maxForce_ * signal_strength * RSinAlphaHSinBeta;
	joint_->addTorque(torque * rotationSign_, maxJointAngularSpeed_ * rotationSign_);

	// compute energy consumption
	float usedEnergy = maxForce_ * signal_strength * BodyConst::MuscleEnergyConstant * dt;
	consumeEnergy(usedEnergy);
}
