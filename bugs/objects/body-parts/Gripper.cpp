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

Gripper::Gripper(BodyPart* parent, float radius, float density, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_GRIPPER, props)
	, radius(radius)
	, active(false)
	, groundJoint(nullptr)
{
	b2CircleShape shape;
	shape.m_radius = radius;
	b2FixtureDef fdef;
	fdef.density = density;
	fdef.friction = 0.3f;
	fdef.restitution = 0.2f;
	fdef.shape = &shape;
	body_->CreateFixture(&fdef);
}

Gripper::~Gripper() {
	setActive(false);
}

void Gripper::setActive(bool active) {
	if (this->active == active)
		return;
	this->active = active;
	if (active) {
		b2WeldJointDef jd;
		jd.bodyA = World::getInstance()->getGroundBody();
		jd.localAnchorA = body_->GetWorldPoint(b2Vec2_zero);
		jd.bodyB = body_;
		groundJoint = (b2WeldJoint*)World::getInstance()->getPhysics()->CreateJoint(&jd);
	} else {
		body_->GetWorld()->DestroyJoint(groundJoint);
		groundJoint = nullptr;
	}
}
