/*
 * Muscle.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 *
 *
 *  Muscle's insertion points are dictated by the genes;
 *  Muscle length is determined by the distance between insertion points at max vs min stretch angles:
 *  	D(phi) = distance between insertion points (I1 and I2) at joint angle phi
 *  	D(phi) = |I1 - I2| where I1 is the insertion point on the left of the joint and I2 on the right, both expressed in joint's space
 *  	D(phi) = sqrt(dx^2 + dy^2) where:
 *  		dx = R1 + Rj - R1*cos(i1) + (R2+Rj)*cos(phi) - R2*cos(phi - i2)
 *  		dy =         - R1*sin(i1) + (R2+Rj)*sin(phi) - R2*sin(phi - i2)
 *  	Dmax = D(phiMin) = distance between insertion points at min angle
 *  	Dmin = D(phiMax) = dist between insertion points at max angle
 *  	for right hand muscle the Dmax and Dmin are reversed
 *  	if Dmax/Dmin < r (r=MuscleContractionRatio) then we need to add a tendon
 *  		(in order to benefit from a shorter, more powerfull muscle that can still cover the required range)
 *  		t (tendon length) = Dmin - (Dmax-Dmin)/(r-1)
 *  		lMin (muscle min length) = Dmin - t
 *  		lMax (muscle max length) = r * lMin = DMax - t = r*(Dmin - t)
 *  	if Dmax/Dmin >= r no tendon is needed, but the muscle range will not be used fully
 *  		(the muscle will not be at its shortest length when the joint is at max angle)
 *  		lMax = DMax
 *  		lMin = lMax/r < Dmin
 *  	in both cases lMin=muscle max contracted length, lMax = muscle relaxed length
 *  Muscle's mass and density are directly set by genes
 *  Muscle's aspect ratio is determined based on mass and relaxed length:
 *  	Asp = lMax^2 * Density / Mass
 *  Muscle width is determined from its relaxed length and aspect ratio
 *  	MuscleWidth = Mass / (lMax * Density)
 *  Muscle width along with muscle density determine its max effective force
 *  Muscle width is considered in relaxed state, since after contraction it will be increased (but this does not lead to an increased force)
 *  max muscle contraction (%) is constant -> longer muscle's length during a full contraction varies more.
 *  muscle contraction speed (m/s) is constant => a longer muscle (which needs to vary its length more) is slower.
 *  muscle knows (by constructor) in which direction it actuates the joint motor.
 *  muscle command signal is clamped to [0.0 : 1.0]
 *  joint motor speed is muscle's max angular speed
 *  joint max torque is signal.value * muscle's max force * lateral offset for the current angle
 *  lateral offset:
 *  	H(phi) = cross(norm(I2(phi)-Cj), norm(I1-Cj)) / D(phi)
 *  	H(phi) must be clamped on the low side in order to avoid the muscle pulling in the wrong direction when joint is angled too much
 *  	min value for H(phi) = EPS + min(R1 * sin(i1), R2 * sin(i2)), i1 and i2 are clamped to [MinMuscleInsertOffset, MaxMuscleInsertOffset]
 *
 *  H(phi) is precomputed at commit time and stored in a table for quick look-up
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

//#define MUSCLE_DRAW_SIMPLIFIED

static const glm::vec3 debug_color(1.f,0.2f, 0.6f);
static const glm::vec3 debug_color_r(0.6f,0.2f, 1.f);

Muscle::Muscle(BodyPartContext const& context, BodyCell& cell, bool isRightSide)
	: BodyPart(BodyPartType::MUSCLE, context, cell, true) // suppress physical body
	, inputSocket_(new InputSocket(nullptr, 1.f))
	, aspectRatio_(0)
	, rotationSign_(isRightSide == cell.isMirrored() ? +1 : -1)
	, maxForce_(0)
	, maxJointAngularSpeed_(0)
	, H_phi_{0}
	, phiAngleStep_(0)
	, cachedPhiMin_(0)
#ifdef DEBUG_DRAW_MUSCLE
	, phiToL_{0}
	, isRightSide_(isRightSide)
#endif
{
	auto &mapMuscleAttr = isRightSide ? cell.mapRightMuscleAttribs_ : cell.mapLeftMuscleAttribs_;
	insertionAngle_[0] = mapMuscleAttr[GENE_MUSCLE_ATTR_INSERT_OFFSET1].clamp(
				BodyConst::MinMuscleInsertionOffset,
				BodyConst::MaxMuscleInsertionOffset);
	insertionAngle_[1] = mapMuscleAttr[GENE_MUSCLE_ATTR_INSERT_OFFSET2].clamp(
				BodyConst::MinMuscleInsertionOffset,
				BodyConst::MaxMuscleInsertionOffset);
	inputVMSCoord_ = cell.mapAttributes_[GENE_ATTRIB_VMS_COORD1].clamp(0, BodyConst::MaxVMSCoordinateValue);

	// insertion angle values from genes are positive, but we need to interpret them in the local context (joint side and mirrored?)
	if (rotationSign_ > 0)
		insertionAngle_[1] *= -1;
	else
		insertionAngle_[0] *= -1;

	float mass = cell.muscleMass(isRightSide);
	float density = BodyConst::MuscleDensity;
	overrideSizeAndDensity(mass / density, density);

	World::getInstance().queueDeferredAction([this] {
		updateFixtures();
	});

	context_.updateList.add(this);

	onDied.add([this](BodyPart*) {
		context_.updateList.remove(this);
	});
}

Muscle::~Muscle() {
	delete inputSocket_;
}

void Muscle::setJoint(JointPivot* joint) {
	assert(!joint_ && "only call this once per instance!");
	assert(joint && "invalid arg (null)");
	joint_ = joint;
	joint_->onDied.add(std::bind(&Muscle::onJointDied, this, std::placeholders::_1));
}

void Muscle::onJointDied(BodyPart* joint) {
	assertDbg(joint == joint_);
	joint_ = nullptr;
	context_.updateList.remove(this);
	// TODO create fixture?
}

void Muscle::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (!joint_) {
		// no joint
		cachedPhiMin_ = 0;
		// TODO must still set aspect ratio and width, length - maybe create a fixture?
		return;
	}

	BodyPart* left = joint_->getLeftAnchor();
	BodyPart* right = joint_->getRightAnchor();
	auto leftTr = left->getWorldTransformation();
	auto rightTr = right->getWorldTransformation();

	float alphaJ1 = pointDirection(vec3xy(joint_->getWorldTransformation() - leftTr)) - leftTr.z; // joint angle in left cell's coordinates
	float alphaJ2 = pointDirection(vec3xy(joint_->getWorldTransformation() - rightTr)) - rightTr.z; // joint angle in right cell's coordinates

	glm::vec2 I1 = left->getAttachmentPoint(alphaJ1 + insertionAngle_[0]); // first insertion point in left cell's space
	anchor_ = {I1, alphaJ1};
	I1 = joint_->worldToLocal(left->localToWorld(I1)); // transform into joint's space

	glm::vec2 I2zero = right->getAttachmentPoint(alphaJ2 + insertionAngle_[1]); // second insertion point in right cell's space at rest angle
	I2zero = joint_->worldToLocal(right->localToWorld(I2zero)); // transform into joint's space

	const auto I2fn = [&, this] (float phi) {	// second insertion point in joint's space as a function of joint angle
		return glm::rotate(I2zero, phi);
	};

	// linear distance function of joint angle (between I1 and I2)
	const auto Dfn = [&, this] (float phi) {
		return clamp(glm::length(I2fn(phi) - I1), EPS, INF);
	};

	// muscle leverage (perpendicular distance from muscle fibre line to joint center) as function of joint angle
	const auto Hfn = [&, this] (float phi) {
		return clamp(cross2D(I1, I2fn(phi)) / Dfn(phi), EPS, INF);
	};

	float phiMin = joint_->getLowerLimit();
	float phiMax = joint_->getUpperLimit();
	cachedPhiMin_ = phiMin;
	float DMin = Dfn(rotationSign_ < 0 ? phiMin : phiMax);
	float DMax = Dfn(rotationSign_ < 0 ? phiMax : phiMin);
	assert(DMax >= DMin);

	// here we compute the characteristics of the muscle
	float r = BodyConst::MuscleContractionRatio;
	// compute tendon if needed
	float t = 0;
	if (DMax / DMin < r)
		t = DMin - (DMax-DMin) / (r-1); // tendon length
	float lMax = DMax - t;
	float lMin = lMax / r;

	aspectRatio_ = sqr(lMax) / size_;
	float w0 = size_ / lMax;	// relaxed muscle length

	maxForce_ = w0 * BodyConst::MuscleForcePerWidthRatio;

	// now compute the leverage (Hfn) table for nAngleSteps intermediate steps:
	phiAngleStep_ = (phiMax - phiMin) / nAngleSteps;
	for (int i=0; i<nAngleSteps; i++) {
		float phi = phiMin + phiAngleStep_ * i;
		H_phi_[i] = Hfn(phi);
#ifdef DEBUG_DRAW_MUSCLE
		phiToL_[i] = clamp(Dfn(phi) - t, 0.f, INF);
#endif
	}

	// must also compute max speed:
	maxJointAngularSpeed_ = joint_->getTotalRange() / (lMax - lMin) * BodyConst::MuscleMaxLinearContractionSpeed;
}

glm::vec2 Muscle::getAttachmentPoint(float relativeAngle) {
	throw std::runtime_error("Should not be called");
}

void Muscle::draw(RenderContext const& ctx) {
#ifdef DEBUG_DRAW_MUSCLE
	if (isDead() || !joint_)
		return;
	float l = lerp_lookup(phiToL_, nAngleSteps, getCurrentPhiSlice());
	float crtAspect = sqr(l) / size_;	// squeeze
	float w = sqrtf(size_ / crtAspect);
	glm::vec3 worldTransform = getWorldTransformation();
#ifdef MUSCLE_DRAW_SIMPLIFIED
	Shape3D::get()->drawLine(
			{vec3xy(worldTransform), 0},
			{vec3xy(worldTransform) + glm::rotate(glm::vec2{l, 0}, worldTransform.z), 0},
			isRightSide_ ? debug_color_r : debug_color);
#else
	Shape3D::get()->drawRectangleXOYCentered(vec3xy(worldTransform) + glm::rotate(glm::vec2{l/2, 0}, worldTransform.z),
			glm::vec2(l, w), worldTransform.z,
			isRightSide_ ? debug_color_r : debug_color);
#endif
#endif
}

float Muscle::getCurrentPhiSlice() {
	if (!joint_)
		return 0;
	float angle = joint_->getJointAngle();
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
	float Hphi = lerp_lookup(H_phi_, nAngleSteps, getCurrentPhiSlice());
	float torque = maxForce_ * Hphi * signal_strength;
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

glm::vec3 Muscle::getWorldTransformation() const {
	if (!joint_)
		return anchor_;
	return joint_->getLeftAnchor()->localToWorld(anchor_);
}
