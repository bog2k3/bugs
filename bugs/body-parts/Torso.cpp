/*
 * Torso.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "Torso.h"
#include "BodyConst.h"
#include "Mouth.h"
#include "../math/math2D.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/RenderContext.h"
#include "../utils/UpdateList.h"
#include "../utils/log.h"
#include "../utils/assert.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(1.f, 1.f, 0.f);

Torso::Torso()
	: BodyPart(BODY_PART_TORSO, std::make_shared<BodyPartInitializationData>())
	, fatMass_(0)
	, lastCommittedTotalSizeInv_(0)
	, frameUsedEnergy_(0)
	, energyBuffer_(0)
	, maxEnergyBuffer_(0)
	, cachedMassTree_(0)
	, extraMass_(0)
	, mouth_(nullptr)
	, foodProcessingSpeed_(0)
	, foodBufferSize_(0)
	, foodBuffer_(0)
{
	physBody_.userObjectType_ = ObjectTypes::BPART_TORSO;
	physBody_.userPointer_ = this;
	physBody_.categoryFlags_ = EventCategoryFlags::BODYPART;
}

Torso::~Torso() {
	if (getUpdateList())
		getUpdateList()->remove(this);
}

void Torso::onAddedToParent() {
	assert(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}

void Torso::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
	}

	float fakeFatMass = fatMass_ + extraMass_;
	float fatSize = (fakeFatMass) * BodyConst::FatDensityInv;
	lastCommittedTotalSizeInv_ = 1.f / (size_ + fatSize);

	float density = (size_ * BodyConst::TorsoDensity + fakeFatMass) / (size_ + fakeFatMass * BodyConst::FatDensityInv);

	// create fixture....
	b2CircleShape shape;
	shape.m_p.Set(0, 0);
	shape.m_radius = sqrtf((size_+fatSize)/PI);

	b2FixtureDef fixDef;
	fixDef.density = density;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);
	maxEnergyBuffer_ = size_ * BodyConst::TorsoEnergyDensity;
	foodProcessingSpeed_ = size_ * BodyConst::FoodProcessingSpeedDensity;
	foodBufferSize_ = foodProcessingSpeed_; // enough for 1 second

#warning "gresit, copiii inca nu sunt scalati"
	cachedMassTree_ = BodyPart::getMass_tree();
}

void Torso::draw(RenderContext const& ctx) {
	if (committed_) {
		// nothing, physics draws
#ifdef DEBUG_DRAW_TORSO
		// draw the inner circle - actual torso size without fat
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
#endif
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
		ctx.shape->drawCircle(pos, sqrtf((size_+(fatMass_+extraMass_)*BodyConst::FatDensityInv)*PI_INV), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(size_/PI), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 Torso::getChildAttachmentPoint(float relativeAngle)
{
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	float fatSize = (fatMass_+extraMass_)*BodyConst::FatDensityInv;
	float fullSize = size_ + fatSize;
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(fullSize * PI_INV), 0), relativeAngle));
	assert(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

void Torso::setInitialFatMass(float fat) {
	assert(!committed_);
	fatMass_ = fat;
}

void Torso::consumeEnergy(float amount) {
	frameUsedEnergy_ += amount;
}

void Torso::update(float dt) {
	energyBuffer_ -= frameUsedEnergy_;
	if (energyBuffer_ < 0) {
		// must use up some fat to compensate since our energy buffer is empty
		float requiredEnergy = -energyBuffer_;
		fatMass_ -= requiredEnergy * BodyConst::FatEnergyDensityInv;
		energyBuffer_ = 0;
		if (fatMass_ > 0) {
			// fill up energy buffer as well
			float fatMassToUse = min(fatMass_, maxEnergyBuffer_ * BodyConst::FatEnergyDensityInv);
			fatMass_ -= fatMassToUse;
			energyBuffer_ += fatMassToUse * BodyConst::FatEnergyDensity;
		}
		float crtSize = fatMass_ * BodyConst::FatDensityInv + size_;
		if (crtSize * lastCommittedTotalSizeInv_ > BodyConst::SizeThresholdToCommit
				|| crtSize * lastCommittedTotalSizeInv_ < BodyConst::SizeThresholdToCommit_inv) {
			commit();
			reattachChildren();
		}
	}
	frameUsedEnergy_ = 0;
	float processedFood = min(foodBuffer_, foodProcessingSpeed_*dt);
	foodBuffer_ -= processedFood;
	onFoodProcessed.trigger(processedFood);
}

void Torso::die() {
	commit();
}

float Torso::addFood(float mass) {
	float amountDeduced = min(mass, foodBufferSize_ - foodBuffer_);
	foodBuffer_ += amountDeduced;
	return amountDeduced;
}

void Torso::replenishEnergyFromMass(float mass) {
	float massToFillBuffer = (maxEnergyBuffer_ - energyBuffer_) * BodyConst::FatEnergyDensityInv;
	float massUsed = min(mass, massToFillBuffer);
	energyBuffer_ += massUsed * BodyConst::FatEnergyDensity;
	mass -= massUsed;
	if (mass > 0) {
		// this mass will be stored as fat
		fatMass_ += mass;
	}
}
