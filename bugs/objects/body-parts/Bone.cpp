/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include <Box2D/Box2D.h>

Bone::Bone(World* world, glm::vec2 position, float rotation, float density, glm::vec2 size, glm::vec2 initialVelocity, float initialAngularVelocity)
	: WorldObject(world, position, rotation, true, initialVelocity, initialAngularVelocity)
{
	b2PolygonShape shape;
	shape.SetAsBox(size.x * 0.5f, size.y * 0.5f);
	b2FixtureDef fixDef;
	fixDef.density = density;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body->CreateFixture(&fixDef);
}

Bone::~Bone() {
}
