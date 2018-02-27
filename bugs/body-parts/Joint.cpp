/*
 * Joint.cpp
 *
 *  Created on: Feb 20, 2018
 *      Author: bog
 */

#include "Joint.h"

#include <boglfw/World.h>
#include <boglfw/math/box2glm.h>
#include <boglfw/math/math3D.h>
#include <boglfw/physics/PhysDestroyListener.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/assert.h>

#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

Joint::Joint(BodyPartContext const& context, BodyCell& cell, BodyPart* leftAnchor, BodyPart* rightAnchor, BodyPartType type)
	: BodyPart(type, context, cell, true)
	, leftAnchor_(leftAnchor)
	, rightAnchor_(rightAnchor)
	, physJoint_(nullptr)
{
	assert(type == BodyPartType::JOINT_PIVOT || type == BodyPartType::JOINT_WELD);
	World::getInstance().queueDeferredAction([this] {
		updateFixtures();
	});
}

Joint::~Joint() {
	if (physJoint_) {
		destroyPhysJoint();
	}
}

void Joint::onPhysJointDestroyed(b2Joint* joint) {
	physJoint_ = nullptr;
}

void Joint::destroyPhysJoint() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	World::getInstance().getDestroyListener()->removeCallback(physJoint_, jointListenerHandle_);
	physJoint_->GetBodyA()->GetWorld()->DestroyJoint(physJoint_);
	physJoint_ = nullptr;
}

/*glm::vec3 Joint::getWorldTransformation() const {
	if (physJoint_) {
		float angle = 0;
#warning "compute angle from anchorB - anchorA"
		return glm::vec3(b2g(physJoint_->GetAnchorA()+physJoint_->GetAnchorB())*0.5f, angle);
	} else
		throw std::runtime_error("This should never happen");
}*/

void Joint::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (physJoint_) {
		destroyPhysJoint();
		physJoint_ = nullptr;
	}

	auto bodyA = leftAnchor_->getBody().b2Body_;
	auto bodyB = rightAnchor_->getBody().b2Body_;

	float radius = sqrtf(size_*PI_INV);
	assert(type_ != BodyPartType::JOINT_WELD || radius == 0);
	float dir = pointDirection(b2g(bodyB->GetPosition() - bodyA->GetPosition()));

	glm::vec2 localAnchorA = leftAnchor_->getAttachmentPoint(dir - bodyA->GetAngle());
	float leftAnchorLength = glm::length(localAnchorA);
	localAnchorA *= 1 + radius/leftAnchorLength;	// move away from the edge by joint radius

	glm::vec2 localAnchorB = rightAnchor_->getAttachmentPoint(dir + PI - bodyB->GetAngle());
	float rightAnchorLength = glm::length(localAnchorB);
	localAnchorB *= 1 + radius/rightAnchorLength;	// move away from the edge by joint radius

	// make sure we set the two anchors in the same position to avoid static stress on the structure
	auto lTransform = leftAnchor_->getWorldTransformation();
	auto wldAnchorA = glm::rotate(localAnchorA, lTransform.z) + vec3xy(lTransform);
	auto rTransform = rightAnchor_->getWorldTransformation();
	auto wldAnchorB = glm::rotate(localAnchorB, rTransform.z) + vec3xy(rTransform);
	auto wldAnchor = (wldAnchorA + wldAnchorB) * 0.5f;
	localAnchorA = glm::rotate(wldAnchor - vec3xy(lTransform), -lTransform.z);
	localAnchorB = glm::rotate(wldAnchor - vec3xy(rTransform), -rTransform.z);

	b2JointDef *def = createJointDef(g2b(localAnchorA), g2b(localAnchorB), bodyB->GetAngle() - bodyA->GetAngle());
	def->bodyA = bodyA;
	def->bodyB = bodyB;
	def->userData = (void*)this;

	physJoint_ = World::getInstance().getPhysics()->CreateJoint(def);
	delete def;
	jointListenerHandle_ = World::getInstance().getDestroyListener()->addCallback(physJoint_,
			std::bind(&Joint::onPhysJointDestroyed, this, std::placeholders::_1));
}
