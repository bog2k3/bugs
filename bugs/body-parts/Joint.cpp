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
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

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
	phiMin_ = initData->phiMin.clamp(-PI*0.9f, limitAngle(initData->phiMin, PI*0.9f));
	phiMax_ = initData->phiMax.clamp(-PI*0.9f, limitAngle(initData->phiMax, PI*0.9f));
	if (phiMin_ > phiMax_) {
		phiMin_ = phiMax_ = (phiMin_ + phiMax_) * 0.5f;
	}
	resetTorque_ = initData->resetTorque.clamp(0, 1.e3f);
}

Joint::Joint()
	: BodyPart(BODY_PART_JOINT, std::make_shared<JointInitializationData>())
	, physJoint_(nullptr)
	, phiMin_(0)
	, phiMax_(0)
	, resetTorque_(0)
{
	auto initData = std::dynamic_pointer_cast<JointInitializationData>(getInitializationData());
	registerAttribute(GENE_ATTRIB_JOINT_LOW_LIMIT, initData->phiMin);
	registerAttribute(GENE_ATTRIB_JOINT_HIGH_LIMIT, initData->phiMax);
	registerAttribute(GENE_ATTRIB_JOINT_RESET_TORQUE, initData->resetTorque);

	getUpdateList()->add(this);
}

Joint::~Joint() {
	if (committed_) {
#warning "must make sure this won't crash (if body doesn't exit any more):"
		if (parent_)
			parent_->getBody().b2Body_->GetWorld()->DestroyJoint(physJoint_);
	}
}

void Joint::commit() {
	assert(nChildren_ == 1);

	if (committed_) {
		physJoint_->GetBodyA()->GetWorld()->DestroyJoint(physJoint_);
		physJoint_ = nullptr;
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
}

glm::vec3 Joint::getWorldTransformation() {
	if (!committed_)
		return BodyPart::getWorldTransformation();
	else {
		return glm::vec3(b2g(physJoint_->GetAnchorA()+physJoint_->GetAnchorB())*0.5f,
			physJoint_->GetBodyA()->GetAngle() + physJoint_->GetReferenceAngle() + physJoint_->GetJointAngle());
	}
}

void Joint::draw(RenderContext const& ctx) {
#ifndef DEBUG_DRAW_JOINT
	if (committed_) {
		// nothing, physics draws
	} else
#endif
	{
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_*PI_INV), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(size_*PI_INV), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 Joint::getChildAttachmentPoint(float relativeAngle)
{
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle));
	assert(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

float Joint::getJointAngle() {
	if (committed_) {
		return physJoint_->GetJointAngle();
	} else
		return 0;
}

void Joint::addTorque(float t, float maxSpeed) {
	vecTorques.push_back(std::make_pair(t, maxSpeed));
}

void Joint::update(float dt) {
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

	// apply the torque and max speed:
	physJoint_->SetMotorSpeed(speed);
	physJoint_->SetMaxMotorTorque(abs(torque));

	// reset pending torques
	vecTorques.clear();

	// add the reset torque if joint is not within epsilon of repause angle:
	if (!eqEps(getJointAngle(), 0, PI/32)) {
		addTorque(-resetTorque_*sign(getJointAngle()), -4*getJointAngle());
	}
}

void Joint::die() {
	physJoint_->EnableMotor(false);
}
