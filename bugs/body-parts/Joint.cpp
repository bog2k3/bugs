/*
 * Joint.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#include "Joint.h"
#include "BodyConst.h"
#include "../World.h"
#include "../math/box2glm.h"
#include "../math/math2D.h"
#include "../renderOpenGL/Shape2D.h"
#include "../utils/log.h"
#include "../utils/assert.h"
#include "../utils/UpdateList.h"
#include "../PhysDestroyListener.h"

#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>
#include <sstream>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static const glm::vec3 debug_color(1.f, 0.3f, 0.1f);

JointInitializationData::JointInitializationData()
	: phiMin(BodyConst::initialJointMinPhi)
	, phiMax(BodyConst::initialJointMaxPhi)
	, resetTorque(BodyConst::initialJointResetTorque) {
	size.reset(BodyConst::initialJointSize);
}

void Joint::cacheInitializationData() {
	BodyPart::cacheInitializationData();
	auto initData = std::dynamic_pointer_cast<JointInitializationData>(getInitializationData());
	phiMin_ = initData->phiMin.clamp(-PI*0.9f, 0);
	phiMax_ = initData->phiMax.clamp(0, limitAngle(initData->phiMax, PI*0.9f));
	resetTorque_ = initData->resetTorque.clamp(0, 1.e3f);
}

Joint::Joint()
	: BodyPart(BodyPartType::JOINT, std::make_shared<JointInitializationData>())
	, physJoint_(nullptr)
	, phiMin_(0)
	, phiMax_(0)
	, resetTorque_(0)
{
	auto initData = std::dynamic_pointer_cast<JointInitializationData>(getInitializationData());
	registerAttribute(GENE_ATTRIB_JOINT_LOW_LIMIT, initData->phiMin);
	registerAttribute(GENE_ATTRIB_JOINT_HIGH_LIMIT, initData->phiMax);
	registerAttribute(GENE_ATTRIB_JOINT_RESET_TORQUE, initData->resetTorque);
}

Joint::~Joint() {
	if (committed_ && physJoint_) {
		destroyPhysJoint();
	}
	if (getUpdateList())
		getUpdateList()->remove(this);
}

void Joint::onAddedToParent() {
	assertDbg(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}

void Joint::commit() {
	assertDbg(nChildren_ == 1);

	if (committed_) {
		destroyPhysJoint();
	}

	b2RevoluteJointDef def;
	def.bodyA = parent_->getBody().b2Body_;
	def.bodyB = children_[0]->getBody().b2Body_;
	def.enableLimit = true;
	def.lowerAngle = phiMin_;
	def.upperAngle = phiMax_;
	def.userData = (void*)this;
	def.enableMotor = true;
	def.referenceAngle = getDefaultAngle() + children_[0]->getDefaultAngle();

	float radius = sqrtf(size_*PI_INV);
	glm::vec2 parentAnchor = parent_->getChildAttachmentPoint(attachmentDirectionParent_);
	float parentAnchorLength = glm::length(parentAnchor);
	parentAnchor *= 1 + radius/parentAnchorLength;	// move away from the edge by joint radius
	def.localAnchorA = g2b(parentAnchor);

	glm::vec2 childAnchor = children_[0]->getChildAttachmentPoint(PI - children_[0]->getAngleOffset());
	float childAnchorLength = glm::length(childAnchor);
	childAnchor *= 1 + radius/childAnchorLength;
	def.localAnchorB = g2b(childAnchor);

	//def.collideConnected = true;

	physJoint_ = (b2RevoluteJoint*)World::getInstance()->getPhysics()->CreateJoint(&def);
	jointListenerHandle_ = World::getInstance()->getDestroyListener()->addCallback(physJoint_,
			std::bind(&Joint::onPhysJointDestroyed, this, std::placeholders::_1));
}

void Joint::destroyPhysJoint() {
	World::getInstance()->getDestroyListener()->removeCallback(physJoint_, jointListenerHandle_);
	physJoint_->GetBodyA()->GetWorld()->DestroyJoint(physJoint_);
	physJoint_ = nullptr;
}

glm::vec3 Joint::getWorldTransformation() {
	if (!committed_)
		return BodyPart::getWorldTransformation();
	else if (physJoint_) {
		return glm::vec3(b2g(physJoint_->GetAnchorA()+physJoint_->GetAnchorB())*0.5f,
			physJoint_->GetBodyA()->GetAngle() + physJoint_->GetReferenceAngle() + physJoint_->GetJointAngle());
	} else
		return glm::vec3(0);
}

void Joint::draw(RenderContext const& ctx) {
#ifndef DEBUG_DRAW_JOINT
	if (committed_) {
		// nothing, physics draws
	} else
#endif
	{
		if (!physJoint_)
			return;
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		if (isDead()) {
			float sizeLeft = getFoodValue() / density_;
			ctx.shape->drawCircle(pos, sqrtf(sizeLeft*PI_INV), 0, 12, glm::vec3(0.5f,0,1));
		} else {
			ctx.shape->drawCircle(pos, sqrtf(size_*PI_INV), 0, 12, debug_color);
			ctx.shape->drawLine(pos,
					pos + glm::rotate(glm::vec2(sqrtf(size_*PI_INV), 0), transform.z),
					0, debug_color);
		}
	}
}

glm::vec2 Joint::getChildAttachmentPoint(float relativeAngle)
{
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

float Joint::getJointAngle() {
	if (committed_) {
		float ret = physJoint_->GetJointAngle();
		if (std::isnan(ret))
			ret = 0;
		else
			ret = limitAngle(ret, PI);
		return ret;
	} else
		return 0;
}

void Joint::addTorque(float t, float maxSpeed) {
	assertDbg(!std::isnan(t));
	assertDbg(!std::isnan(maxSpeed));
	vecTorques.push_back(std::make_pair(t, maxSpeed));
}

void Joint::update(float dt) {
	if (!physJoint_ || dt == 0)
		return;
	float invdt = 1.f / dt;
	float reactionTorque = physJoint_->GetReactionTorque(invdt);
	float motorTorque = physJoint_->GetMotorTorque(invdt);
	float reactionForce = physJoint_->GetReactionForce(invdt).Length();
	bool jointIsFUBAR = std::isnan(reactionTorque) || std::isnan(reactionForce);
	bool excessForce = reactionForce > size_ * BodyConst::JointForceToleranceFactor;
	bool excessRTorque = abs(reactionTorque) > size_ * BodyConst::JointTorqueToleranceFactor;
	bool excessMTorque = abs(motorTorque) > size_ * BodyConst::JointTorqueToleranceFactor;
#ifdef DEBUG
	if (getDebugName() == "Torso::Joint(8)::Bone(0)::Joint(0)") {
//		LOGLN("FORCE: " << reactionForce << "\t\tMTORQUE: " << motorTorque << "\t\tRTORQUE: " << reactionTorque);
	}
#endif
	if (false)
	if (jointIsFUBAR || excessForce || excessMTorque || excessRTorque) {
		// this joint is toast - must break free the downstream body parts
#ifdef DEBUG
		LOG("JOINT BREAK: " << getDebugName() << " (");
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
		BodyPart* downStream = children_[0];
		downStream->detach(true); // this will be taken over by bug entity
		detach(true);
		destroyPhysJoint();
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
	physJoint_->SetMotorSpeed(speed);
	physJoint_->SetMaxMotorTorque(abs(torque));
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

void Joint::die() {
	if (committed_ && physJoint_)
		physJoint_->EnableMotor(false);
}

void Joint::onDetachedFromParent() {
	/*if (physJoint_) {
		World::getInstance()->getPhysics()->DestroyJoint(physJoint_);
		physJoint_ = nullptr;
	}*/
}

void Joint::onPhysJointDestroyed(b2Joint* joint) {
	physJoint_ = nullptr;
}

