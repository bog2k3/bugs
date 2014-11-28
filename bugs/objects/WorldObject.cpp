/*
 * WorldObject.cpp
 *
 *  Created on: Nov 13, 2014
 *      Author: bog
 */

#include "WorldObject.h"
#include "../World.h"
#include "../math/math2D.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/Viewport.h"
#include "../renderOpenGL/Camera.h"
#include <Box2D/Box2D.h>

WorldObject::WorldObject(World* world, glm::vec2 position, float angle, bool dynamic, glm::vec2 velocity, float angularVelocity)
	: world(world)
	, physics(world->getPhysics())
{
	b2BodyDef def;
	def.angle = angle;
	def.position.Set(position.x, position.y);
	def.type = dynamic ? b2_dynamicBody : b2_staticBody;
	def.userData = (void*)this;
	def.angularDamping = def.linearDamping = 0.3f;

	body = physics->CreateBody(&def);
}

WorldObject::~WorldObject() {
}

