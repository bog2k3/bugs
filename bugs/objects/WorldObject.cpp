/*
 * WorldObject.cpp
 *
 *  Created on: Nov 13, 2014
 *      Author: bog
 */

#include "WorldObject.h"
#include "../math/math.h"
#include "../renderOpenGL/Rectangle.h"
#include "../physics/RigidBody.h"
#include "../physics/Spring.h"

WorldObject::WorldObject(RigidBody* rigid)
	: type(TYPE_RIGID)
	, rigidBody(rigid)
	, spring(nullptr)
	, ownerOfResource(true)
{
}

WorldObject::WorldObject(Spring* spring)
	: type(TYPE_SPRING)
	, rigidBody(nullptr)
	, spring(spring)
	, ownerOfResource(true)
{
}

WorldObject::WorldObject(WorldObjectType type)
	: type(type)
	, rigidBody(nullptr)
	, spring(nullptr)
	, ownerOfResource(false)
{
}

WorldObject::~WorldObject() {
	if (ownerOfResource) {
		if (rigidBody)
			delete rigidBody;
		if (spring)
			delete spring;
	}
}

void WorldObject::draw(ObjectRenderContext* ctx)
{
	switch (type) {
		case TYPE_RIGID: {
			glm::vec2 size = rigidBody->getLocalBoundingBox().getSize();
			ctx->rectangle->draw(rigidBody->getPosition().x, rigidBody->getPosition().y, 0, size.x, size.y, rigidBody->getRotation(), 0, 1, 0);
			break;
		}
		case TYPE_SPRING:
			break;
		default:
			break;
	}
}
