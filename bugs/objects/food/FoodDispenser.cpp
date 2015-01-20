/*
 * FoodDispenser.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#include "FoodDispenser.h"
#include "FoodChunk.h"
#include "../WorldConst.h"
#include <glm/gtx/rotate_vector.hpp>

FoodDispenser::FoodDispenser(glm::vec2 position, float direction)
	: position_(position)
	, direction_(direction)
	, period_(WorldConst::BasicFoodDispenserPeriod)
	, timer_(0.f)
	, spawnPosition_(position + glm::rotate(glm::vec2(WorldConst::BasicFoodDispenserSpawnPositionX, WorldConst::BasicFoodDispenserSpawnPositionY), direction))
	, spawnDirection_(direction)
	, spawnVelocity_(WorldConst::BasicFoodDispenserSpawnVelocity)
	, spawnMass_(WorldConst::BasicFoodDispenserSpawnMass)
{
	PhysicsProperties props(position, direction, false, glm::vec2(0), 0);
	createPhysicsBody(props);
}

FoodDispenser::~FoodDispenser() {
}

void FoodDispenser::draw(RenderContext& ctx) {

}

template<> void update(FoodDispenser*& disp, float dt) {
	disp->update(dt);
}

void FoodDispenser::update(float dt) {
	timer_ += dt;
	if (timer_ > period_) {
		// create one food chunk
		timer_ = 0;
		FoodChunk* f = new FoodChunk(position_+spawnPosition_, spawnDirection_, spawnDirection_*spawnVelocity_, 0, spawnMass_);
		World::getInstance()->addObject(f);
	}
}
