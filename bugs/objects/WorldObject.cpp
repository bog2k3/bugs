/*
 * WorldObject.cpp
 *
 *  Created on: Nov 13, 2014
 *      Author: bog
 */

#include "WorldObject.h"

#include "../math/math2D.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/Viewport.h"
#include "../renderOpenGL/Camera.h"
#include <Box2D/Box2D.h>

WorldObject::WorldObject(b2World* world, glm::vec2 position, float angle, bool dynamic, glm::vec2 velocity, float angularVelocity)
{
	b2BodyDef def;
	def.angle = angle;
	def.position.Set(position.x, position.y);
	def.type = dynamic ? b2_dynamicBody : b2_staticBody;
	def.userData = (void*)this;
	def.angularDamping = def.linearDamping = 0.1f;

	body = world->CreateBody(&def);
}

WorldObject::~WorldObject() {
}

void WorldObject::draw(ObjectRenderContext* ctx)
{
	/*switch (type) {
		case TYPE_RIGID: {
			// glm::vec2 size = rigidBody->getLocalBoundingBox().getSize();
			// ctx->shape->drawRectangle(rigidBody->getPosition(), 0, size, rigidBody->getRotation(), glm::vec3(0,1,0));
			break;
		}
		case TYPE_SPRING: {
			// glm::vec2 size = glm::vec2(10, 10) / ctx->viewport->getCamera()->getZoomLevel();  // always 10 pixels
			// render attachment #1:
			// ctx->shape->drawRectangle(spring->a1.getWorldPos(), 0, size, PI*0.25, glm::vec3(1,0,0));
			// render attachment #2:
			// ctx->shape->drawRectangle(spring->a2.getWorldPos(), 0, size, PI*0.25, glm::vec3(1,0,0));
			// render line:
			// ctx->shape->drawLine(spring->a1.getWorldPos(), spring->a2.getWorldPos(), 0, glm::vec3(1,0,0));
			break;
		}
		default:
			break;
	}*/
}
