/*
 * FoodDispenser.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#include "FoodDispenser.h"
#include "FoodChunk.h"
#include "../WorldConst.h"
#include "../../utils/rand.h"
#include "../../World.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

FoodDispenser::FoodDispenser(glm::vec2 position, float direction)
	: radius_(sqrtf(WorldConst::FoodDispenserSize * PI_INV))
	, position_(position)
	, direction_(direction)
	, period_(WorldConst::FoodDispenserPeriod)
	, timer_(0.f)
	, spawnVelocity_(WorldConst::FoodDispenserSpawnVelocity)
	, spawnMass_(WorldConst::FoodDispenserSpawnMass)
{
	physBody_.userObjectType_ = ObjectTypes::FOOD_DISPENSER;
	physBody_.userPointer_ = this;
	physBody_.categoryFlags_ = EventCategoryFlags::STATIC;

	PhysicsProperties props(position, direction, false, glm::vec2(0), 0);
	physBody_.create(props);

	// create fixture
	b2CircleShape shp;
	shp.m_radius = radius_;
	b2FixtureDef fdef;
	fdef.shape = &shp;
	physBody_.b2Body_->CreateFixture(&fdef);
}

FoodDispenser::~FoodDispenser() {
}

void FoodDispenser::draw(RenderContext const& ctx) {

}

void FoodDispenser::update(float dt) {
	timer_ += dt;
	if (timer_ > period_) {
		// create one food chunk
		timer_ = 0;
		glm::vec2 offset(radius_ * 1.05f, 0);
		float randomAngle = srandf() * WorldConst::FoodDispenserSpreadAngleHalf;
		offset = glm::rotate(offset, direction_ + randomAngle);
		glm::vec2 velocity = glm::normalize(offset) * spawnVelocity_;
		FoodChunk* chunk = new FoodChunk(position_ + offset, direction_+randomAngle, velocity, 0, spawnMass_);
		World::getInstance()->takeOwnershipOf(chunk);
	}
}
