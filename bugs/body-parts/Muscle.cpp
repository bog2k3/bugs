/*
 * Muscle.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 *
 *
 *  Muscle's insertion points are dictated by the genes;
 *  max Muscle length is determined by the distance between insertion points at max stretch angle;
 *  	D(phiMax) = distance between insertion points at max angle
 *  	D(phiMin) = dist between insertion points at min angle
 *  	MaxMuscleLength = D(phiMax)
 *  min Muscle length is either the distance at min angle or the maximum contracted length (whichever is larger)
 *  	MinMuscleLength = max(D(phiMin), MaxMuscleLength * BodyConst::MaxMuscleContractionRatio)
 *  	if the max contracted muscle length is larger than D(phiMin) then the muscle has no further effect on the joint below that angle (its force becomes zero),
 *  	thus it cannot bring the joint to its min angle
 *  Muscle's mass and density are directly set by genes
 *  Muscle's aspect ratio is determined based on mass and length:
 *  	Asp = MaxMuscleLength^2 * Density / Mass
 *  Muscle width is determined from its max length and aspect ratio
 *  	MuscleWidth = Mass / (MaxMuscleLength * Density)
 *  Muscle width along with muscle density determine its max effective force
 *  Muscle width is considered at longest stretch, since after contraction it will be increased (but this does not lead to an increased force)
 *  max muscle contraction (%) is constant -> longer muscle's length during a full contraction varies more.
 *  muscle contraction speed (m/s) is constant => a longer muscle (which needs to vary its length more) is slower.
 *  muscle knows (by constructor) in which direction it actuates the joint motor.
 *  muscle command signal is clamped to [0.0 : 1.0]
 *  joint motor speed is muscle's max angular speed
 *  joint max torque is signal.value * muscle's max force * lateral offset for the current angle
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
#include "Bone.h"
#include "BodyConst.h"
#include "BodyCell.h"
#include "../neuralnet/InputSocket.h"
#include "../entities/Bug.h"

#include <boglfw/World.h>
#include <boglfw/math/math3D.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/assert.h>
#include <boglfw/utils/log.h>
#include <boglfw/perf/marker.h>

#include <glm/vec3.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <math.h>
#include <Box2D/Box2D.h>
#include "JointPivot.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static const glm::vec3 debug_color(1.f,0.2f, 0.8f);

//MuscleInitializationData::MuscleInitializationData()
//	: aspectRatio(BodyConst::initialMuscleAspectRatio) {
//	density.reset(BodyConst::MuscleDensity);
//}
//
//void Muscle::cacheInitializationData() {
//	BodyPart::cacheInitializationData();
//	auto initData = std::dynamic_pointer_cast<MuscleInitializationData>(getInitializationData());
//	aspectRatio_ = initData->aspectRatio.clamp(BodyConst::MaxBodyPartAspectRatioInv, BodyConst::MaxBodyPartAspectRatio);
//}
//
//float Muscle::getInputVMSCoord(unsigned index) const {
//	if (index != 0)
//		return 0;
//	auto initData = std::dynamic_pointer_cast<MuscleInitializationData>(getInitializationData());
//	if (initData)
//		return initData->inputVMSCoord.clamp(0, BodyConst::MaxVMSCoordinateValue);
//	else
//		return 0;
//}

Muscle::Muscle(BodyPartContext const& context, BodyCell& cell, bool isRightSide)
	: BodyPart(BodyPartType::MUSCLE, context, cell, true) // suppress physical body
	, inputSocket_(new InputSocket(nullptr, 1.f))
//	, aspectRatio_(1.f)
	, maxForce_(0)
	, maxJointAngularSpeed_(0)
	, phiToRSinAlphaHSinBeta_{0}
	, phiAngleStep_(0)
	, cachedPhiMin_(0)
#ifdef DEBUG_DRAW_MUSCLE
	, phiToDx_{0}
#endif
{
	auto &mapMuscleAttr = isRightSide ? cell.mapRightMuscleAttribs_ : cell.mapLeftMuscleAttribs_;
	mapMuscleAttr[GENE_MUSCLE_ATTR_ASPECT_RATIO].changeAbs(BodyConst::initialMuscleAspectRatio);
	aspectRatio_ = cell.mapAttributes_[GENE_ATTRIB_ASPECT_RATIO].clamp(
				BodyConst::MaxBodyPartAspectRatioInv,
				BodyConst::MaxBodyPartAspectRatio);
	inputVMSCoord_ = cell.mapAttributes_[GENE_ATTRIB_VMS_COORD1].clamp(0, BodyConst::MaxVMSCoordinateValue);
}

Muscle::~Muscle() {
	delete inputSocket_;
}

void Muscle::setJoint(JointPivot* joint, int motorDirSign) {
	assert(!joint_ && "only call this once per instance!");
	assert(joint && "invalid arg (null)");
	joint_ = joint;
	rotationSign_ = motorDirSign;
	joint_->onDied.add(std::bind(&Muscle::onJointDied, this, std::placeholders::_1));
}

void Muscle::die() {
	context_.updateList.remove(this);
}

void Muscle::onJointDied(BodyPart* joint) {
	assertDbg(joint == joint_);
	joint_ = nullptr;
	context_.updateList.remove(this);
}

/*void Muscle::onAddedToParent() {
	assertDbg(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}*/

void Muscle::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (joint_) {
		// here we compute the characteristics of the muscle
		float w0 = sqrtf(size_ / aspectRatio_); // relaxed width
		float l0 = aspectRatio_ * w0; // relaxed length
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
		BodyPart* targetPart = nullptr; //joint_->getChild(0);
		bool useOY = false;
		if (targetPart->getType() == BodyPartType::BONE) {
			Bone* bone = dynamic_cast<Bone*>(targetPart);
			if (bone->aspectRatio() < 1.f)
				useOY = true;
		}
		// this is the world angle of insertion axis in default joint position:
		float phi0 = targetPart->getWorldTransformation().z + (useOY ? PI/2 : 0) - joint_->getJointAngle();
		glm::vec2 phi0_v(glm::rotate(glm::vec2(1, 0), phi0));

		// compute the muscle angle (phi_muscle):
		glm::vec2 M(vec3xy(getWorldTransformation()));
		glm::vec2 J(vec3xy(joint_->getWorldTransformation()));
		glm::vec2 h_v(J-M);
		float h = glm::length(h_v);
		// at this time, both M, J, h and phi0_v are expressed in world space
		float phi_muscle = acosf(clamp(glm::dot(glm::normalize(-h_v), phi0_v), -1.f, +1.f));
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
		if (D < 0)
			D = 0;
#warning "above if D<0 we should disable this muscle permanently"
		float sqrtD = sqrt(D);
		float bneg = -2*sqr(dx)*h*CM + sqr(dx)*h*p;
		float denom = 1.f / (2*sqr(h*p) - 2*sqr(dx));
		float r1 = (bneg + dx*sqrtD) * denom;
		float r2 = (bneg - dx*sqrtD) * denom;
		float r = r1 < 0 ? r2 : (r2 < 0 ? r1 : min(r1, r2));	// use the smaller non-negative value
		assertDbg(!std::isnan(r));

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
			float t_dot_v = glm::dot(t_v_norm, r_v_norm);
			float alpha = acosf(clamp(t_dot_v, -1.f, 1.f));
			float h_dot_t = glm::dot(h_v_norm, t_v_norm);
			float beta = acosf(clamp(h_dot_t, -1.f, 1.f));
			phiToRSinAlphaHSinBeta_[i] = r * sinf(alpha) + h * sinf(beta);
#ifdef DEBUG_DRAW_MUSCLE
			phiToDx_[i] = t0 - glm::length(h_v + r_v_norm*r);
#endif
		}

		// must also compute max speed:
		maxJointAngularSpeed_ = joint_->getTotalRange() / dx * BodyConst::MuscleMaxLinearContractionSpeed;
	} else {
		// no joint
		cachedPhiMin_ = 0;
	}
}

glm::vec2 Muscle::getAttachmentPoint(float relativeAngle) {
	throw std::runtime_error("Should not be called");
	/*float w = sqrtf(size_ / aspectRatio_);
	float l = aspectRatio_ * w;
	glm::vec2 ret(rayIntersectBox(l, w, relativeAngle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;*/
}

void Muscle::draw(RenderContext const& ctx) {
	float crtAspect = aspectRatio_;
#ifdef DEBUG_DRAW_MUSCLE
	if (isDead())
		return;
	float w0 = sqrtf(size_ / aspectRatio_);
	float l0 = aspectRatio_ * w0;
	float dx = lerp_lookup(phiToDx_, nAngleSteps, getCurrentPhiSlice());
	crtAspect *= sqr((l0 - dx) / l0);	// squeeze
#endif
	float w = sqrtf(size_ / crtAspect);
	float l = crtAspect * w;
	glm::vec3 worldTransform = getWorldTransformation();
//	Shape3D::get()->drawRectangleXOYCentered(vec3xy(worldTransform),
//			glm::vec2(l, w), worldTransform.z, debug_color);
//	Shape3D::get()->drawLine(
//			{vec3xy(worldTransform), 0},
//			{vec3xy(worldTransform) + glm::rotate(getAttachmentPoint(0), worldTransform.z), 0},
//			debug_color);
#ifdef DEBUG_DRAW_MUSCLE
	if (inputSocket_->value > 0)
		Shape3D::get()->drawLine(
			{vec3xy(worldTransform) + glm::rotate(glm::vec2(-l/2, 0), worldTransform.z + PI/2), 0},
			{vec3xy(worldTransform) + glm::rotate(glm::vec2(+l/2, 0), worldTransform.z + PI/2), 0},
			debug_color);
#endif
}

float Muscle::getCurrentPhiSlice() {
	float angle = joint_ ? joint_->getJointAngle() : 0;
	float angleSlice = (angle - cachedPhiMin_) / phiAngleStep_;
	int iAngleSlice = (int) angleSlice;
	angleSlice -= iAngleSlice;
	if (iAngleSlice >= nAngleSteps)
		iAngleSlice = nAngleSteps - 1;
	if (iAngleSlice < 0)
		iAngleSlice = 0;
	float ret = iAngleSlice + angleSlice;
	assertDbg(!std::isnan(ret));
	return ret;
}

void Muscle::update(float dt) {
	PERF_MARKER_FUNC;
	if (isDead())
		return;
	float signal_strength = clamp(inputSocket_->value, 0.f, 1.f);
	assertDbg(!std::isnan(signal_strength));
	float RSinAlphaHSinBeta = lerp_lookup(phiToRSinAlphaHSinBeta_, nAngleSteps, getCurrentPhiSlice());
	float torque = maxForce_ * signal_strength * RSinAlphaHSinBeta;
	if (joint_) {		// joint may have broken
		joint_->addTorque(torque * rotationSign_, maxJointAngularSpeed_ * rotationSign_);
#ifdef DEBUG
//		if (joint_->getDebugName() == "Torso::Joint(8)::Bone(0)::Joint(0)") {
//			LOGLN(getDebugName() << " generate torque " << torque * rotationSign_);
//		}
#endif
	}

	// compute energy consumption
	float usedEnergy = maxForce_ * signal_strength * BodyConst::MuscleEnergyConstant * dt;
	context_.owner.consumeEnergy(usedEnergy);
}
