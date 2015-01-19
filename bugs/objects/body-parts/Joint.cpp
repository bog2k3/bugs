/*
 * Joint.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#include "Joint.h"
#include "BodyConst.h"
#include "../../World.h"
#include "../../math/box2glm.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../log.h"
#include "../../UpdateList.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(1.f, 0.3f, 0.1f);

JointInitializationData::JointInitializationData()
	: phiMin(BodyConst::initialJointMinPhi)
	, phiMax(BodyConst::initialJointMaxPhi)
	, resetTorque(BodyConst::initialJointResetTorque) {
	size.reset(BodyConst::initialJointSize);
}

Joint::Joint(BodyPart* parent)
	: BodyPart(parent, BODY_PART_JOINT, std::make_shared<JointInitializationData>())
	, jointInitialData_(std::static_pointer_cast<JointInitializationData>(getInitializationData()))
	, physJoint_(nullptr)
	, repauseAngle_(0)
	, resetTorque_(0)
{
	std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();
	registerAttribute(GENE_ATTRIB_JOINT_LOW_LIMIT, initData->phiMin);
	registerAttribute(GENE_ATTRIB_JOINT_HIGH_LIMIT, initData->phiMax);
	registerAttribute(GENE_ATTRIB_JOINT_RESET_TORQUE, initData->resetTorque);

	getUpdateList()->add(this);
}

Joint::~Joint() {
	// delete joint
	//...

	getUpdateList()->remove(this);
}

/**
 * if the angles are screwed, limit them to [-PI/9, 0] (low) and [0, +PI/9] (high)
 */
void Joint::getNormalizedLimits(float &low, float &high) {
	std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();
	low = limitAngle(initData->phiMin, 0);
	if (low < -PI*0.9f)
		low = -PI*0.9f;
	high = limitAngle(initData->phiMax, PI*0.9f);
	if (high < 0)
		high = 0;
}

void Joint::commit() {
	assert(nChildren_ == 1);

	if (committed_) {
		physJoint_->GetBodyA()->GetWorld()->DestroyJoint(physJoint_);
		physJoint_ = nullptr;
	}

	std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();

	float lowAngle, highAngle;
	getNormalizedLimits(lowAngle, highAngle);

	b2RevoluteJointDef def;
	def.bodyA = parent_->getBody();
	def.bodyB = children_[0]->getBody();
	def.enableLimit = true;
	def.lowerAngle = lowAngle;
	def.upperAngle = highAngle;
	def.userData = (void*)this;
	def.enableMotor = true;
	def.referenceAngle = getDefaultAngle() + children_[0]->getDefaultAngle();

	float radius = sqrtf(getInitializationData()->size/PI);
	glm::vec2 parentAnchor = parent_->getChildAttachmentPoint(getInitializationData()->attachmentDirectionParent);
	float parentAnchorLength = glm::length(parentAnchor);
	parentAnchor *= 1 + radius/parentAnchorLength;	// move away from the edge by joint radius
	def.localAnchorA = g2b(parentAnchor);

	glm::vec2 childAnchor = children_[0]->getChildAttachmentPoint(PI - children_[0]->getInitializationData()->angleOffset);
	float childAnchorLength = glm::length(childAnchor);
	childAnchor *= 1 + radius/childAnchorLength;
	def.localAnchorB = g2b(childAnchor);

	//def.collideConnected = true;

	physJoint_ = (b2RevoluteJoint*)World::getInstance()->getPhysics()->CreateJoint(&def);

	repauseAngle_ = initData->angleOffset;
	resetTorque_ = initData->resetTorque;
}

glm::vec3 Joint::getWorldTransformation() const {
	if (!committed_)
		return BodyPart::getWorldTransformation();
	else {
		return glm::vec3(b2g(physJoint_->GetAnchorA()+physJoint_->GetAnchorB())*0.5f,
			physJoint_->GetBodyA()->GetAngle() + physJoint_->GetReferenceAngle() + physJoint_->GetJointAngle());
	}
}

void Joint::draw(RenderContext& ctx) {
#ifndef DEBUG_DRAW_JOINT
	if (committed_) {
		// nothing, physics draws
	} else
#endif
	{
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(getInitializationData()->size/PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(getInitializationData()->size/PI), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 Joint::getChildAttachmentPoint(float relativeAngle) const
{
	assert(getInitializationData());
	return glm::rotate(glm::vec2(sqrtf(getInitializationData()->size * PI_INV), 0), relativeAngle);
}

float Joint::getLowerLimit() {
	if (committed_) {
		return physJoint_->GetLowerLimit();
	} else {
		std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();
		return max(min(initData->phiMin.get(), 0.f), -PI*0.9f);
	}
}

float Joint::getUpperLimit() {
	if (committed_) {
		return physJoint_->GetUpperLimit();
	} else {
		std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();
		return min(max(initData->phiMax.get(), 0.f), PI*0.9f);
	}
}

float Joint::getTotalRange() {
	if (committed_) {
		return physJoint_->GetUpperLimit() - physJoint_->GetLowerLimit();
	} else {
		std::shared_ptr<JointInitializationData> initData = jointInitialData_.lock();
		float lowAngle, highAngle;
		getNormalizedLimits(lowAngle, highAngle);
		// compute range:
		return highAngle - lowAngle;
	}
}

float Joint::getJointAngle() {
	if (committed_)
		return physJoint_->GetJointAngle();
	else
		return 0;
}

void Joint::addTorque(float t, float maxSpeed) {
	vecTorques.push_back(std::make_pair(t, maxSpeed));
}

template<> void update(Joint*& j, float dt) {
	j->update(dt);
}

void Joint::update(float dt) {
	// compute the resulting torque and speed and apply it to the joint
	float minSpeed = 0, maxSpeed = 0;
	float torque = 0;
	for (unsigned i=0; i<vecTorques.size(); i++) {
		torque += vecTorques[i].first;
		if (vecTorques[i].second < minSpeed)
			minSpeed = vecTorques[i].second;
		else if (vecTorques[i].second > maxSpeed)
			maxSpeed = vecTorques[i].second;
	}
	float speed = torque > 0 ? maxSpeed : minSpeed;

	// apply the torque and max speed:
	physJoint_->SetMotorSpeed(speed);
	physJoint_->SetMaxMotorTorque(abs(torque));

	// reset pending torques
	vecTorques.clear();

	// add the reset torque if joint is not within epsilon of repause angle:
	if (!eqEps(getJointAngle(), PI/32)) {
		addTorque(-resetTorque_*sign(getJointAngle()), -4*getJointAngle());
	}
}

void Joint::die() {
	physJoint_->EnableMotor(false);
}
