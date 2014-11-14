/*
 * WorldObject.cpp
 *
 *  Created on: Nov 13, 2014
 *      Author: bog
 */

#include "WorldObject.h"
#include "../math/math.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/Viewport.h"
#include "../renderOpenGL/Camera.h"
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
			ctx->shape->drawRectangle(rigidBody->getPosition(), 0, size, rigidBody->getRotation(), glm::vec3(0,1,0));
			break;
		}
		case TYPE_SPRING: {
			glm::vec2 size = glm::vec2(10, 10) / ctx->viewport->getCamera()->getZoomLevel();  // always 10 pixels
			// render attachment #1:
			ctx->shape->drawRectangle(spring->a1.getWorldPos(), 0, size, PI*0.25, glm::vec3(1,0,0));
			// render attachment #2:
			ctx->shape->drawRectangle(spring->a2.getWorldPos(), 0, size, PI*0.25, glm::vec3(1,0,0));
			// render line:
			ctx->shape->drawLine(spring->a1.getWorldPos(), spring->a2.getWorldPos(), 0, glm::vec3(1,0,0));
			break;
		}
		default:
			break;
	}
}
