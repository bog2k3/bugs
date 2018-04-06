/*
 * Joint.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#include "JointPivot.h"

#include "BodyConst.h"
#include "BodyCell.h"

#include <boglfw/World.h>
#include <boglfw/math/box2glm.h>
#include <boglfw/math/math3D.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/physics/PhysDestroyListener.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/assert.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/perf/marker.h>

#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>
#include <sstream>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static const glm::vec3 debug_color(1.f, 0.3f, 0.1f);

JointPivot::JointPivot(BodyPartContext const& context, BodyCell& cell, BodyPart* leftAnchor, BodyPart* rightAnchor)
	: Joint(context, cell, leftAnchor, rightAnchor, BodyPartType::JOINT_PIVOT)
{

	phiMin_ = cell.mapJointAttribs_[GENE_JOINT_ATTR_LOW_LIMIT].clamp(-PI*0.9f, 0);
	phiMax_ = cell.mapJointAttribs_[GENE_JOINT_ATTR_HIGH_LIMIT].clamp(0, PI*0.9f);
	if (cell.isMirrored()) {
		xchg(phiMin_, phiMax_);
		phiMin_ *= -1;
		phiMax_ *= -1;
	}
	resetTorque_ = cell.mapJointAttribs_[GENE_JOINT_ATTR_RESET_TORQUE].clamp(0, BodyConst::MaxJointResetTorque);

	context.updateList.add(this);
}

JointPivot::~JointPivot() {
	context_.updateList.remove(this);
}

b2JointDef* JointPivot::createJointDef(b2Vec2 localAnchorA, b2Vec2 localAnchorB, float refAngle) {

	b2RevoluteJointDef* def = new b2RevoluteJointDef();
	def->localAnchorA = localAnchorA;
	def->localAnchorB = localAnchorB;
	def->referenceAngle = refAngle;
	def->enableLimit = true;
	def->lowerAngle = phiMin_;
	def->upperAngle = phiMax_;
	def->enableMotor = true;

	return def;
}

//void JointPivot::destroyPhysJoint() {
//#ifdef DEBUG
//	World::assertOnMainThread();
//#endif
//	World::getInstance().getDestroyListener()->removeCallback(physJoint_, jointListenerHandle_);
//	physJoint_->GetBodyA()->GetWorld()->DestroyJoint(physJoint_);
//	physJoint_ = nullptr;
//}
//
glm::vec3 JointPivot::getWorldTransformation() const {
	if (!physJoint_) {
		return {0.f, 0.f, 0.f};
	}
	auto anchorA = physJoint_->GetAnchorA();
	float angle = pointDirection(b2g(anchorA - physJoint_->GetBodyA()->GetPosition())) + b2PJoint()->GetJointAngle();
	return {b2g(anchorA), angle};
}

void JointPivot::draw(RenderContext const& ctx) {
#ifdef DEBUG_DRAW_JOINT
	glm::vec3 transform = getWorldTransformation();
	glm::vec2 pos = vec3xy(transform);
	if (isDead()) {
		float sizeLeft = getFoodValue() / density_;
		Shape3D::get()->drawCircleXOY(pos, sqrtf(sizeLeft*PI_INV), 12, glm::vec3(0.5f,0,1));
	} else {
		Shape3D::get()->drawCircleXOY(pos, sqrtf(size_*PI_INV), 12, debug_color);
		Shape3D::get()->drawLine({pos, 0},
				{pos + glm::rotate(glm::vec2(sqrtf(size_*PI_INV), 0), transform.z), 0},
				debug_color);
	}
#endif
}

float JointPivot::getJointAngle() const {
	if (physJoint_) {
		float ret = b2PJoint()->GetJointAngle();
		if (std::isnan(ret))
			ret = 0;
		else
			ret = limitAngle(ret, PI);
		return ret;
	} else
		return 0;
}

void JointPivot::addTorque(float t, float maxSpeed) {
	assertDbg(!std::isnan(t));
	assertDbg(!std::isnan(maxSpeed));
	vecTorques.push_back(std::make_pair(t, maxSpeed));
}

void JointPivot::update(float dt) {
	PERF_MARKER_FUNC;
	if (!physJoint_ || dt == 0)
		return;
	float invdt = 1.f / dt;
	float reactionTorque = b2PJoint()->GetReactionTorque(invdt);
	float motorTorque = b2PJoint()->GetMotorTorque(invdt);
	float reactionForce = b2PJoint()->GetReactionForce(invdt).Length();
	bool jointIsFUBAR = std::isnan(reactionTorque) || std::isnan(reactionForce);
	bool excessForce = reactionForce > size_ * density_ * BodyConst::JointForceToleranceFactor;
	bool excessRTorque = abs(reactionTorque) > size_ * density_ * BodyConst::JointTorqueToleranceFactor;
	bool excessMTorque = abs(motorTorque) > size_ * density_ * BodyConst::JointTorqueToleranceFactor;
#ifdef DEBUG
//	if (getOwner()->getId() == 1) {
//		if (getDebugName() == "Torso::Joint(8)::Bone(0)::Joint(0)" ||
//				getDebugName() == "Torso::Joint(8)") {
//			LOGLN(getDebugName() << " FORCE: " << reactionForce << "\t\tMTORQUE: " << motorTorque << "\t\tRTORQUE: " << reactionTorque);
//		}
//	}
#endif
	if (jointIsFUBAR || excessForce || excessMTorque || excessRTorque) {
		// this joint is toast - must break free the downstream body parts
#ifdef DEBUG
		LOG("JOINT BREAK: " << /*getDebugName() <<*/ " (");
		std::stringstream reason;
		if (jointIsFUBAR)
			reason << "FUBAR";
		else if (excessForce)
			reason << "EXCESS-FORCE: " << reactionForce << " [max:" << size_ * BodyConst::JointForceToleranceFactor<< "]";
		else if (excessMTorque)
			reason << "EXCESS-MTORQUE: " << motorTorque << " [max:" << size_ * BodyConst::JointTorqueToleranceFactor<< "]";
		else if (excessRTorque)
			reason << "EXCESS-RTORQUE: " << reactionTorque << " [max:" << size_ * BodyConst::JointTorqueToleranceFactor<< "]";
		LOGNP(reason.str() << ")\n");

#endif
		World::getInstance().queueDeferredAction([this] () {
			destroyPhysJoint();
		});
		return;
	}

	if (isDead())
		return;

	// compute the resulting torque and speed and apply it to the joint
	float minSpeed = 0, maxSpeed = 0;
	float torque = 0;
	for (unsigned i=0; i<vecTorques.size(); i++) {
		if (eqEps(vecTorques[i].first, 0, 1.e-5f))
			continue;
		torque += vecTorques[i].first;
		if (vecTorques[i].second < minSpeed)
			minSpeed = vecTorques[i].second;
		else if (vecTorques[i].second > maxSpeed)
			maxSpeed = vecTorques[i].second;
	}
	float speed = torque > 0 ? maxSpeed : minSpeed;

	assertDbg(!std::isnan(speed));
	assertDbg(!std::isnan(torque));

	// apply the torque and max speed:
	b2PJoint()->SetMotorSpeed(speed);
	b2PJoint()->SetMaxMotorTorque(abs(torque));
	// reset pending torques
	vecTorques.clear();

	// add the reset torque if joint is not within epsilon of repause angle:
	if (!eqEps(getJointAngle(), 0, PI/32)) {
		float reset_speed = -4*getJointAngle();
		addTorque(-resetTorque_*sign(getJointAngle()), reset_speed);
		if (abs(reset_speed) > 200)
			LOGLN("reset speed: "<<reset_speed);
	}
}

void JointPivot::die() {
	if (physJoint_)
		b2PJoint()->EnableMotor(false);
}

/*void Joint::onDetachedFromParent() {
	if (physJoint_) {
		World::getInstance().getPhysics()->DestroyJoint(physJoint_);
		physJoint_ = nullptr;
	}
}*/

float JointPivot::getDensity(BodyCell const& cell) {
	auto it = cell.mapJointAttribs_.find(GENE_JOINT_ATTR_DENSITY);
	auto value = it != cell.mapJointAttribs_.end() ? it->second : CumulativeValue();
	value.changeAbs(BodyConst::initialJointDensity);
	return value.clamp(BodyConst::MinBodyPartDensity, BodyConst::MaxBodyPartDensity);
}

b2RevoluteJoint* JointPivot::b2PJoint() const {
	return static_cast<b2RevoluteJoint*>(physJoint_);
}
