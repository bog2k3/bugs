/*
 * FoodChunk.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#include "FoodChunk.h"
#include "../WorldConst.h"
#include "../../math/math2D.h"
#include <Box2D/Box2D.h>

FoodChunk::FoodChunk(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass)
	: size_(mass * WorldConst::FoodChunkDensityInv)
	, amountLeft_(mass)
{
	PhysicsProperties props(position, angle, true, velocity, angularVelocity);
	createPhysicsBody(props);

	// now create the fixture
	b2CircleShape shp;
	shp.m_radius = sqrtf(size_ * PI_INV);
	b2FixtureDef fdef;
	fdef.density = WorldConst::FoodChunkDensity;
	fdef.friction = 0.2f;
	fdef.restitution = 0.3f;
	fdef.shape = &shp;
	body_->CreateFixture(&fdef);
}

FoodChunk::~FoodChunk() {
	body_->DestroyFixture(&body_->GetFixtureList()[0]);
}

void FoodChunk::draw(RenderContext& ctx) {
	// TODO put a sign of amountLeft on it
}
