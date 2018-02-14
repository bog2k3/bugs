/*
 * FatCell.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: bog
 */

#include "FatCell.h"
#include "BodyConst.h"
#include "BodyCell.h"
#include "../ObjectTypesAndFlags.h"

#include <boglfw/World.h>

#include <Box2D/Box2D.h>

#include <glm/gtx/rotate_vector.hpp>

FatCell::~FatCell() {
	// TODO Auto-generated destructor stub
}

FatCell::FatCell(BodyPartContext const& context, BodyCell& cell)
	: BodyPart(BodyPartType::FAT, context, cell)
	, frameUsedEnergy_(0)
	, energyBuffer_(0)
	, foodBuffer_(0)
{
	maxEnergyBuffer_ = size_ * BodyConst::FatEnergyDensity;
	foodProcessingSpeed_ = size_ * BodyConst::FoodProcessingSpeedDensity;
	foodBufferSize_ = foodProcessingSpeed_; // enough for 1 second

	physBody_.userObjectType_ = ObjectTypes::BPART_FATCELL;
	physBody_.userPointer_ = this;
}

float FatCell::getDensity(BodyCell const& cell) {
	return BodyConst::FatDensity;
}

static glm::vec2 getFatCellAttachmentPoint(float size, float angle) {
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size * PI_INV), 0), angle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

glm::vec2 FatCell::getAttachmentPoint(float relativeAngle) {
	return getFatCellAttachmentPoint(size_, relativeAngle);
}

float FatCell::getRadius(BodyCell const& cell, float angle) {
	glm::vec2 p = getFatCellAttachmentPoint(cell.size(), angle);
	return glm::length(p);
}

float FatCell::consumeEnergy(float amount) {
	frameUsedEnergy_ += amount;
}

void FatCell::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	float fRatio;
	auto fSize = adjustFixtureValues({size_, 0.f}, fRatio);

	// create fixture....
	b2CircleShape shape;
	shape.m_p.Set(0, 0);
	shape.m_radius = sqrtf((fSize.first)/PI);

	b2FixtureDef fixDef;
	fixDef.density = density_ / fRatio;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);
}
