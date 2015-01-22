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
	, lifeTime_(0)
{
	PhysicsProperties props(position, angle, true, velocity, angularVelocity);
	physBody_.create(props);

	// now create the fixture
	b2CircleShape shp;
	shp.m_radius = sqrtf(size_ * PI_INV);
	b2FixtureDef fdef;
	fdef.density = WorldConst::FoodChunkDensity;
	fdef.friction = 0.2f;
	fdef.restitution = 0.3f;
	fdef.shape = &shp;
	physBody_.b2Body_->CreateFixture(&fdef);
}

FoodChunk::~FoodChunk() {
	onDestroy.trigger(this);
}

void FoodChunk::draw(RenderContext const& ctx) {
	// TODO put a sign of amountLeft on it
}

void FoodChunk::update(float dt) {
	lifeTime_ += dt;
	if (lifeTime_ >= WorldConst::FoodChunkLifeTime) {
		destroy();
	}
}
