/*
 * FoodChunk.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#include "FoodChunk.h"
#include "../WorldConst.h"
#include "../../ObjectTypesAndFlags.h"

#include <boglfw/World.h>
#include <boglfw/math/math3D.h>
#include <boglfw/math/aabb.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/perf/marker.h>

#include <Box2D/Box2D.h>

FoodChunk::FoodChunk(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass)
	: physBody_(ObjectTypes::FOOD_CHUNK, this, EventCategoryFlags::FOOD, 0)
	, size_(mass * WorldConst::FoodChunkDensityInv)
	, initialMass_(mass)
	, amountLeft_(mass)
{
	PhysicsProperties props(position, angle, true, velocity, angularVelocity);

	World::getInstance().queueDeferredAction([this, props]() {
		physBody_.create(props);
		physBody_.getEntityFunc_ = &getEntityFromFoodChunkPhysBody;

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
	});
}

FoodChunk::~FoodChunk() {
	onDestroy.trigger(this);
}

#ifdef DEBUG_DRAW_FOOD_CHUNK
void FoodChunk::draw(Viewport* vp) {
	Shape3D::get()->drawCircleXOY(physBody_.getPosition(),
			sqrtf(amountLeft_.load(std::memory_order_relaxed)*PI_INV*WorldConst::FoodChunkDensityInv),
					8, glm::vec3(1.f, 0.5f, 0.f));
}
#endif

void FoodChunk::update(float dt) {
	PERF_MARKER_FUNC;
	consume(dt * WorldConst::FoodChunkDecaySpeed);
}

void FoodChunk::consume(float massAmount) {
	float prev = amountLeft_.load(std::memory_order_relaxed);
	while (!amountLeft_.compare_exchange_weak(prev, prev-massAmount, std::memory_order_acq_rel, std::memory_order_relaxed))
		; // loop
	if (prev > 0 && prev <= massAmount)
		destroy();
}

//glm::vec3 FoodChunk::getWorldTransform() const {
//	if (physBody_.b2Body_) {
//		auto pos = physBody_.b2Body_->GetPosition();
//		return glm::vec3(b2g(pos), physBody_.b2Body_->GetAngle());
//	} else
//		return glm::vec3(0);
//}

Entity* FoodChunk::getEntityFromFoodChunkPhysBody(PhysicsBody const& body) {
	FoodChunk* pChunk = static_cast<FoodChunk*>(body.userPointer_);
	assertDbg(pChunk);
	return pChunk;
}

aabb FoodChunk::getAABB(bool requirePrecise) const {
	return physBody_.getAABB();
}
