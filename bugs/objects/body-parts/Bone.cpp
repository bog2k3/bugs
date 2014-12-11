/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include <Box2D/Box2D.h>

Bone::Bone(BodyPart* parent, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_BONE, props)
	, density_(1)
	, size_(1.e-4f, 0.7f)	// 1 sq cm, more wide than long
	, committed_(false)
{
}

Bone::~Bone() {
}

void Bone::setDensity(float value) {
	assert(!committed_);
	density_ = value;
}
void Bone::setSize(glm::vec2 value) {
	assert(!committed_);
	size_ = value;
}
void Bone::commit() {
	assert(!committed_);
	// preprocess...
	// right now size_ contains x=surface_area and y=aspect_ratio=length/width
	// transform to width, length
	size_.x = sqrtf(size_.x/size_.y);
	size_.y *= size_.x;

	// create fixture:
	b2PolygonShape shape;
	shape.SetAsBox(size_.x * 0.5f, size_.y * 0.5f);
	b2FixtureDef fixDef;
	fixDef.density = density_;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);

	committed_ = true;
}
