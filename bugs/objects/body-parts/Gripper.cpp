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

Gripper::Gripper(World* world, glm::vec2 position, float radiud, float density)
	: WorldObject(world, position, 0, true, glm::vec2(0), 0)
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
	body->CreateFixture(&fdef);
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
		jd.bodyA = getWorld()->getGroundBody();
		jd.localAnchorA = body->GetWorldPoint(b2Vec2_zero);
		jd.bodyB = body;
		groundJoint = (b2WeldJoint*)getPhysics()->CreateJoint(&jd);
	} else {
		body->GetWorld()->DestroyJoint(groundJoint);
		groundJoint = nullptr;
	}
}
