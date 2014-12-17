/*
 * Gripper.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#include "Gripper.h"
#include "../../World.h"
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>

Gripper::Gripper(BodyPart* parent, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_GRIPPER, props)
	, radius_(0.01f)
	, density_(1.f)
	, active_(false)
	, groundJoint_(nullptr)
{
}

Gripper::~Gripper() {
	setActive(false);
}

void Gripper::commit() {
	assert(!committed_);
	b2CircleShape shape;
	shape.m_radius = radius_;
	b2FixtureDef fdef;
	fdef.density = density_;
	fdef.friction = 0.3f;
	fdef.restitution = 0.2f;
	fdef.shape = &shape;
	body_->CreateFixture(&fdef);
}

void Gripper::setRadius(float value) {
	assert(!committed_);
	radius_ = value;
}
void Gripper::setDensity(float value) {
	assert(!committed_);
	density_ = value;
}

void Gripper::setActive(bool active) {
	if (active_ == active)
		return;
	active_ = active;
	if (active) {
		b2WeldJointDef jd;
		jd.bodyA = World::getInstance()->getGroundBody();
		jd.localAnchorA = body_->GetWorldPoint(b2Vec2_zero);
		jd.bodyB = body_;
		groundJoint_ = (b2WeldJoint*)World::getInstance()->getPhysics()->CreateJoint(&jd);
	} else {
		body_->GetWorld()->DestroyJoint(groundJoint_);
		groundJoint_ = nullptr;
	}
}
