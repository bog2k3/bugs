/*
 * PhysicsBody.cpp
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#include "PhysicsBody.h"
#include "../World.h"
#include "../math/box2glm.h"
#include <Box2D/Box2D.h>

void PhysicsBody::create(const PhysicsProperties& props) {
	assert(b2Body_==nullptr);

	b2BodyDef def;
	def.angle = props.angle;
	def.position.Set(props.position.x, props.position.y);
	def.type = props.dynamic ? b2_dynamicBody : b2_staticBody;
	def.userData = (void*)this;
	def.angularDamping = def.linearDamping = 0.3f;
	def.angularVelocity = props.angularVelocity;
	def.linearVelocity = g2b(props.velocity);

	b2Body_ = World::getInstance()->getPhysics()->CreateBody(&def);
}

PhysicsBody::~PhysicsBody() {
	if (b2Body_)
		b2Body_->GetWorld()->DestroyBody(b2Body_);
}
