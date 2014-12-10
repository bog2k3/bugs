/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include <Box2D/Box2D.h>

Bone::Bone(BodyPart* parent, float density, glm::vec2 size, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_BONE, props)
{
	b2PolygonShape shape;
	shape.SetAsBox(size.x * 0.5f, size.y * 0.5f);
	b2FixtureDef fixDef;
	fixDef.density = density;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);
}

Bone::~Bone() {
}
