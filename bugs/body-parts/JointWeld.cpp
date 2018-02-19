/*
 * JointWeld.cpp
 *
 *  Created on: Dec 31, 2017
 *      Author: bog
 */

#include "JointWeld.h"

#include <boglfw/World.h>
#include <boglfw/math/box2glm.h>
#include <boglfw/math/math3D.h>
#include <boglfw/physics/PhysDestroyListener.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/assert.h>
#include <boglfw/renderOpenGL/Shape3D.h>

#include <Box2D/Box2D.h>

JointWeld::JointWeld(BodyPartContext const& context, BodyCell& cell, BodyPart* leftAnchor, BodyPart* rightAnchor)
	: BodyPart(BodyPartType::JOINT_WELD, context, cell, true)
	, leftAnchor_(leftAnchor)
	, rightAnchor_(rightAnchor)
	, physJoint_(nullptr)
{
}

JointWeld::~JointWeld() {
	if (physJoint_) {
		destroyPhysJoint();
	}
}

void JointWeld::onPhysJointDestroyed(b2Joint* joint) {
	physJoint_ = nullptr;
}

void JointWeld::draw(RenderContext const& ctx) {
#ifndef DEBUG_DRAW_JOINT
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

void JointWeld::destroyPhysJoint() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	World::getInstance().getDestroyListener()->removeCallback(physJoint_, jointListenerHandle_);
	physJoint_->GetBodyA()->GetWorld()->DestroyJoint(physJoint_);
	physJoint_ = nullptr;
}

glm::vec3 JointWeld::getWorldTransformation() const {
	if (physJoint_) {
		float angle = 0;
#warning "compute angle from anchorB - anchorA"
		return glm::vec3(b2g(physJoint_->GetAnchorA()+physJoint_->GetAnchorB())*0.5f, angle);
	} else
		throw std::runtime_error("This should never happen");
}

void JointWeld::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (physJoint_) {
		destroyPhysJoint();
		physJoint_ = nullptr;
	}

	b2WeldJointDef def;
	def.bodyA = leftAnchor_->getBody().b2Body_;
	def.bodyB = rightAnchor_->getBody().b2Body_;
	def.userData = (void*)this;
	def.referenceAngle = 0; // TODO fix this

	// TODO determine angles for anchor points
	glm::vec2 leftAnchorPoint = leftAnchor_->getAttachmentPoint(0);
	def.localAnchorA = g2b(leftAnchorPoint);

	// TODO fix angle for anchor points
	glm::vec2 rightAnchorPoint = rightAnchor_->getAttachmentPoint(PI - 0);
	def.localAnchorB = g2b(rightAnchorPoint);

	//def.collideConnected = true;

	physJoint_ = (b2WeldJoint*)World::getInstance().getPhysics()->CreateJoint(&def);
	jointListenerHandle_ = World::getInstance().getDestroyListener()->addCallback(physJoint_,
			std::bind(&JointWeld::onPhysJointDestroyed, this, std::placeholders::_1));
}
