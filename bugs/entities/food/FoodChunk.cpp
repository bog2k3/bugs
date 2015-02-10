/*
 * FoodChunk.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#include "FoodChunk.h"
#include "../WorldConst.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include "../../renderOpenGL/Shape2D.h"
#include <Box2D/Box2D.h>

FoodChunk::FoodChunk(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass)
	: physBody_(ObjectTypes::FOOD_CHUNK, this, EventCategoryFlags::FOOD, 0)
	, size_(mass * WorldConst::FoodChunkDensityInv)
	, initialMass_(mass)
	, amountLeft_(mass)
{
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
	fdefK.filter.groupIndex = b2FilterGroup::FOOD_CHUNK;
	physBody_.b2Body_->CreateFixture(&fdefK);
}

FoodChunk::~FoodChunk() {
	onDestroy.trigger(this);
}

#ifdef DEBUG_DRAW_FOOD_CHUNK
void FoodChunk::draw(RenderContext const& ctx) {
	ctx.shape->drawCircle(physBody_.getPosition(),
			sqrtf(amountLeft_*PI_INV*WorldConst::FoodChunkDensityInv),
					0, 8, glm::vec3(1.f, 0.5f, 0.f));
}
#endif

void FoodChunk::update(float dt) {
	consume(dt * WorldConst::FoodChunkDecaySpeed);
}

void FoodChunk::consume(float massAmount) {
	amountLeft_ -= massAmount;
	if (amountLeft_ <= 0)
		destroy();
}