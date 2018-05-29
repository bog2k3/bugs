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
#include <boglfw/renderOpenGL/Shape3D.h>

#include <Box2D/Box2D.h>

#include <glm/gtx/rotate_vector.hpp>

FatCell::~FatCell() {
	// TODO Auto-generated destructor stub
}

FatCell::FatCell(BodyPartContext const& context, BodyCell& cell)
	: BodyPart(BodyPartType::FAT, context, cell)
	, frameUsedEnergy_(0)
	, energyBuffer_(0)
{
	maxEnergyBuffer_ = size_ * BodyConst::FatEnergyBufferDensity;

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
	float consumedFromBuffer = min(energyBuffer_, amount);
	float totalConsumed = consumedFromBuffer;
	if (consumedFromBuffer < amount && size_ > 0) {
		// buffer is depleted, must use fat mass to refill
		float massRequired = BodyConst::FatEnergyDensityInv * (maxEnergyBuffer_ + amount - consumedFromBuffer);
		float sizeRequired = massRequired * BodyConst::FatDensityInv;
		float sizeUsed = min(sizeRequired, size_);
		float sizeDiff = sizeUsed;
		float convertedEnergy = sizeUsed * BodyConst::FatDensity * BodyConst::FatEnergyDensity;
		float extraConsumed = min(amount - consumedFromBuffer, convertedEnergy);
		totalConsumed += extraConsumed;
		energyBuffer_ += convertedEnergy - extraConsumed;

		applyScale((size_ - sizeDiff) / size_);
	}
	return totalConsumed;
}

void FatCell::replenishFromMass(float mass) {
	float massToFillBuffer = (maxEnergyBuffer_ - energyBuffer_) * BodyConst::FatEnergyDensityInv;
	float massUsed = min(mass, massToFillBuffer);
	energyBuffer_ += massUsed * BodyConst::FatEnergyDensity;
	mass -= massUsed;
	if (mass > 0) {
		// this mass will be stored as fat
		float sizeDiff = mass / BodyConst::FatDensity;
		applyScale((size_ + sizeDiff) / size_);
	}
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

void FatCell::draw(Viewport* vp) {
	if (!isDead())
		return;
	glm::vec3 transform = getWorldTransformation();
	glm::vec3 pos = {vec3xy(transform), 0};
	float sizeLeft = getFoodValue() / density_;
	Shape3D::get()->drawCircleXOY(pos, sqrtf(sizeLeft*PI_INV)*0.6f, 12, glm::vec3(0.5f,0,1));
}
