/*
 * JointWeld.cpp
 *
 *  Created on: Dec 31, 2017
 *      Author: bog
 */

#include "JointWeld.h"
#include "BodyConst.h"

#include <boglfw/World.h>
#include <boglfw/math/box2glm.h>
#include <boglfw/math/math3D.h>
#include <boglfw/physics/PhysDestroyListener.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/assert.h>
#include <boglfw/renderOpenGL/Shape3D.h>

#include <Box2D/Box2D.h>

#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(1.f, 0.3f, 0.1f);
#define DEBUG_DRAW_JOINT

JointWeld::JointWeld(BodyPartContext const& context, BodyCell& cell, BodyPart* leftAnchor, BodyPart* rightAnchor)
	: Joint(context, cell, leftAnchor, rightAnchor, BodyPartType::JOINT_WELD)
{
}

JointWeld::~JointWeld() {
}

void JointWeld::draw(RenderContext const& ctx) {
#ifdef DEBUG_DRAW_JOINT
	glm::vec3 transform = getWorldTransformation();
	glm::vec2 pos = vec3xy(transform);
	float angle = transform.z;
	float o1 = 0.05f * sqrt(leftAnchor_->size());
	float o2 = o1 * 0.4f;
	Shape3D::get()->drawLine({pos - glm::rotate(glm::vec2(o1, 0), angle), 0},
		{pos + glm::rotate(glm::vec2(o1, 0), angle), 0},
		debug_color);
	Shape3D::get()->drawLine({pos - glm::rotate(glm::vec2(0, o2), angle), 0},
		{pos + glm::rotate(glm::vec2(0, o2), angle), 0},
		debug_color);
#endif
}

glm::vec3 JointWeld::getWorldTransformation() const {
	if (!physJoint_) {
		return {0.f, 0.f, 0.f};
	}
	auto anchorA = physJoint_->GetAnchorA();
	float angle = pointDirection(b2g(physJoint_->GetBodyB()->GetPosition() - physJoint_->GetBodyA()->GetPosition()));
	return {b2g(anchorA), angle};
}

//void JointWeld::updateFixtures() {
//#ifdef DEBUG
//	World::assertOnMainThread();
//#endif
//	if (physJoint_) {
//		destroyPhysJoint();
//		physJoint_ = nullptr;
//	}
//
//	b2WeldJointDef def;
//	def.bodyA = leftAnchor_->getBody().b2Body_;
//	def.bodyB = rightAnchor_->getBody().b2Body_;
//	def.userData = (void*)this;
//	def.referenceAngle = 0; // TODO fix this
//
//	// TODO determine angles for anchor points
//	glm::vec2 leftAnchorPoint = leftAnchor_->getAttachmentPoint(0);
//	def.localAnchorA = g2b(leftAnchorPoint);
//
//	// TODO fix angle for anchor points
//	glm::vec2 rightAnchorPoint = rightAnchor_->getAttachmentPoint(PI - 0);
//	def.localAnchorB = g2b(rightAnchorPoint);
//
//	//def.collideConnected = true;
//
//	physJoint_ = (b2WeldJoint*)World::getInstance().getPhysics()->CreateJoint(&def);
//	jointListenerHandle_ = World::getInstance().getDestroyListener()->addCallback(physJoint_,
//			std::bind(&JointWeld::onPhysJointDestroyed, this, std::placeholders::_1));
//}

b2JointDef* JointWeld::createJointDef(b2Vec2 localAnchorA, b2Vec2 localAnchorB, float refAngle) {
	b2WeldJointDef *def = new b2WeldJointDef();
	def->localAnchorA = localAnchorA;
	def->localAnchorB = localAnchorB;
	def->referenceAngle = refAngle;
	def->dampingRatio = 0.99f;
	def->frequencyHz = 20;

	return def;
}

float JointWeld::breakForce() const {
	float averageAnchorSize = (leftAnchor_->mass() + rightAnchor_->mass()) * 0.5f;
	return averageAnchorSize * BodyConst::JointForceToleranceFactor;
}

float JointWeld::breakTorque() const {
	float averageAnchorSize = (leftAnchor_->mass() + rightAnchor_->mass()) * 0.5f;
	return averageAnchorSize * BodyConst::JointTorqueToleranceFactor;
}
