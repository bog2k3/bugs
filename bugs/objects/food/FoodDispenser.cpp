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
	: direction_(direction)
	, period_(WorldConst::BasicFoodDispenserPeriod)
	, timer_(0.f)
	, spawnPosition_(position + glm::rotate(WorldConst::BasicFoodDispenserSpawnPosition, )
	, spawnDirection_
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
		FoodChunk* f = new FoodChunk(pos, angle, velo, angVel, mass);
		World::getInstance()->addObject(f);
	}
}
