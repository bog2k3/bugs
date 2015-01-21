/*
 * FoodDispenser.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#include "FoodDispenser.h"
#include "FoodChunk.h"
#include "../WorldConst.h"
#include "../../math/tools.h"
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
	PhysicsProperties props(position, direction, false, glm::vec2(0), 0);
	createPhysicsBody(props);

	// create fixture
	b2CircleShape shp;
	shp.m_radius = radius_;
	b2FixtureDef fdef;
	fdef.shape = &shp;
	body_->CreateFixture(&fdef);
}

FoodDispenser::~FoodDispenser() {
	onDestroy.trigger(this);
}

void FoodDispenser::draw(RenderContext& ctx) {

}

void FoodDispenser::update(float dt) {
	timer_ += dt;
	updateList_.update(dt);
	if (timer_ > period_) {
		// create one food chunk
		timer_ = 0;
		glm::vec2 offset(radius_ * 1.05f, 0);
		float randomAngle = srandf() * WorldConst::FoodDispenserSpreadAngleHalf;
		offset = glm::rotate(offset, direction_ + randomAngle);
		glm::vec2 velocity = glm::normalize(offset) * spawnVelocity_;
		updateList_.add(new FoodChunk(position_ + offset, direction_+randomAngle, velocity, 0, spawnMass_));
	}
}
