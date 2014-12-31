/*
 * WorldObject.cpp
 *
 *  Created on: Nov 13, 2014
 *      Author: bog
 */

#include "WorldObject.h"
#include "../World.h"
#include "../math/math2D.h"
#include "../math/box2glm.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/Viewport.h"
#include "../renderOpenGL/Camera.h"
#include <Box2D/Box2D.h>

WorldObject::WorldObject()
	: body_(nullptr)
{
	World::getInstance()->addObject(this);
}

WorldObject::~WorldObject() {
	World::getInstance()->removeObject(this);
}

void WorldObject::createPhysicsBody(PhysicsProperties const &props) {
	assert(body_==nullptr);

	b2BodyDef def;
	def.angle = props.angle;
	def.position.Set(props.position.x, props.position.y);
	def.type = props.dynamic ? b2_dynamicBody : b2_staticBody;
	def.userData = (void*)this;
	def.angularDamping = def.linearDamping = 0.3f;
	def.angularVelocity = props.angularVelocity;
	def.linearVelocity = g2b(props.velocity);

	body_ = World::getInstance()->getPhysics()->CreateBody(&def);
}
