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

WorldObject::WorldObject(PhysicsProperties props, bool autoCreatePhysicsBody/*=false*/)
	: body_(nullptr)
	, initialData_(new PhysicsProperties(props))
{
	World::getInstance()->addObject(this);
	if (autoCreatePhysicsBody)
		createPhysicsBody();
}

WorldObject::~WorldObject() {
	World::getInstance()->removeObject(this);
}

void WorldObject::createPhysicsBody() {
	assert(body_==nullptr);

	b2BodyDef def;
	def.angle = initialData_->angle;
	def.position.Set(initialData_->position.x, initialData_->position.y);
	def.type = initialData_->dynamic ? b2_dynamicBody : b2_staticBody;
	def.userData = (void*)this;
	def.angularDamping = def.linearDamping = 0.3f;
	def.angularVelocity = initialData_->angularVelocity;
	def.linearVelocity = g2b(initialData_->velocity);

	body_ = World::getInstance()->getPhysics()->CreateBody(&def);
}

void WorldObject::purgeInitializationData() {
	// delete the initialization data after creating the body since it's not needed any more
	delete initialData_;
	initialData_ = nullptr;
}
