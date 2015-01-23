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
	, initialMass_(mass)
	, amountLeft_(mass)
{
	physBody_.userObjectType_ = ObjectTypes::FOOD_CHUNK;
	physBody_.userPointer_ = this;
	physBody_.categoryFlags_ = CategoryFlags::FOOD;

	PhysicsProperties props(position, angle, true, velocity, angularVelocity);
	physBody_.create(props);

	// now create the sensor fixture
	b2CircleShape shp;
	shp.m_radius = sqrtf(size_ * WorldConst::FoodChunkSensorRatio * PI_INV);
	b2FixtureDef fdef;
	fdef.density = 0;
	fdef.shape = &shp;
	fdef.isSensor = true;
	physBody_.b2Body_->CreateFixture(&fdef);

	// and the kernel fixture:
	shp.m_radius = sqrtf(size_ * PI_INV);
	b2FixtureDef fdefK;
	fdefK.density = WorldConst::FoodChunkDensity;
	fdefK.friction = 0.2f;
	fdefK.restitution = 0.3f;
	fdefK.shape = &shp;
	physBody_.b2Body_->CreateFixture(&fdefK);
}

FoodChunk::~FoodChunk() {
	onDestroy.trigger(this);
}

void FoodChunk::draw(RenderContext const& ctx) {
	// TODO put a sign of amountLeft on it
}

void FoodChunk::update(float dt) {
	consume(dt * WorldConst::FoodChunkDecaySpeed);
}

void FoodChunk::consume(float massAmount) {
	amountLeft_ -= massAmount;
	if (amountLeft_ <= 0)
		destroy();
}
