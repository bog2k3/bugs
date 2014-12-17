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

WorldObject::WorldObject(PhysicsProperties props, bool autoCommit/*=false*/)
	: physProps_(new PhysicsProperties(props))
	, committed_(false)
{
	if (autoCommit)
		commit();
}

WorldObject::~WorldObject() {
}

void WorldObject::commit() {
	assert(!committed_);
	committed_ = true;

	b2BodyDef def;
	def.angle = physProps_->angle;
	def.position.Set(physProps_->position.x, physProps_->position.y);
	def.type = physProps_->dynamic ? b2_dynamicBody : b2_staticBody;
	def.userData = (void*)this;
	def.angularDamping = def.linearDamping = 0.3f;
	def.angularVelocity = physProps_->angularVelocity;
	def.linearVelocity = g2b(physProps_->velocity);

	body_ = World::getInstance()->getPhysics()->CreateBody(&def);

	// delete the initialization data since we don't need it any more after this step:
	delete physProps_;
	physProps_ = nullptr;
}
