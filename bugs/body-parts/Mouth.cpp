/*
 * Mouth.cpp
 *
 *  Created on: Jan 18, 2015
 *      Author: bog
 */

#include "Mouth.h"
#include "BodyConst.h"
#include "BodyCell.h"
#include "../entities/food/FoodChunk.h"
#include "../entities/Bug/Bug.h"
#include "../ObjectTypesAndFlags.h"

#include <boglfw/World.h>
#include <boglfw/math/math3D.h>
#include <boglfw/math/box2glm.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/assert.h>
#include <boglfw/perf/marker.h>

#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static const glm::vec3 debug_color(0.2f, 0.8f, 1.f);

#define DEBUG_DRAW_MOUTH

Mouth::Mouth(BodyPartContext const& context, BodyCell& cell)
	: BodyPart(BodyPartType::MOUTH, context, cell)
	, bufferSize_(0)
	, usedBuffer_(0)
{
	float aspectRatio = extractAspectRatio(cell);
	length_ = sqrtf(size_ * aspectRatio);	// l = sqrt(s*a)
	width_ = length_ / aspectRatio;			// w = l/a

	onCollisionEventHandle_ = physBody_.onCollision.add(std::bind(&Mouth::onCollision, this, std::placeholders::_1, std::placeholders::_2));
	physBody_.userObjectType_ = ObjectTypes::BPART_MOUTH;
	physBody_.userPointer_ = this;
	physBody_.collisionEventMask_ = EventCategoryFlags::FOOD;

	context.updateList.add(this);

	onDied.add([this](BodyPart*) {
		context_.updateList.remove(this);
		physBody_.onCollision.remove(onCollisionEventHandle_);
		physBody_.collisionEventMask_ = 0;
		lastDt_ = 0;
	});
}

Mouth::~Mouth() {
}

static glm::vec2 getMouthAttachmentPoint(float length, float width, float angle) {
	glm::vec2 ret(rayIntersectBox(length, width, angle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

glm::vec2 Mouth::getAttachmentPoint(float relativeAngle) {
	return getMouthAttachmentPoint(length_, width_, relativeAngle);
}

void Mouth::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	float fRatio;
	auto fSize = adjustFixtureValues({width_, length_}, fRatio);

	bufferSize_ = size_ * BodyConst::MouthBufferDensity;

	// create fixture:
	b2PolygonShape shape;
	shape.SetAsBox(fSize.second * 0.5f, fSize.first * 0.5f); // our x and y mean length and width, so are reversed (because length is parallel to OX axis)
	b2FixtureDef fixDef;
	fixDef.density = density_ / fRatio;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);
}

void Mouth::draw(Viewport* vp) {
#ifdef DEBUG_DRAW_MOUTH
	float ratio = sqrt(usedBuffer_ / bufferSize_);
	glm::vec3 color(0.8f, 0.5f, 0.f);
	if (isDead()) {
		ratio = sqrt((getFoodValue() / density_) / size_);
		color = glm::vec3(0.5f, 0, 1);
	}
	float widthLeft = width_ * ratio;
	float lengthLeft = length_ * ratio;
	glm::vec3 worldTransform = getWorldTransformation();
	Shape3D::get()->drawRectangleXOYCentered(glm::vec3(vec3xy(worldTransform), 0), glm::vec2(lengthLeft, widthLeft), worldTransform.z, color);
#endif
	BodyPart::draw(vp);
}

void Mouth::onCollision(PhysicsBody* pOther, float impulseMagnitude) {
	if (impulseMagnitude > 1000) {
		//TODO implement hurt
		NOT_IMPLEMENTED;
		return;
	}
	b2AABB otherAABB;
	pOther->b2Body_->GetFixtureList()[0].GetShape()->ComputeAABB(
			&otherAABB,
			pOther->b2Body_->GetTransform(),
			0);
	b2Vec2 otherSize = 2.f * otherAABB.GetExtents();
	float maxSwallowRatio = min(1.f, width_ / max(max(width_, otherSize.x), otherSize.y));
	float maxFoodAvailable = 0;

	// check how much food there is:
	switch (pOther->userObjectType_) {
	case ObjectTypes::FOOD_CHUNK: {
		auto pFoodChunk = static_cast<FoodChunk*>(pOther->userPointer_);
		if (pFoodChunk->isZombie())
			return;
		maxFoodAvailable = pFoodChunk->getMassLeft();
		break;
	}
	case ObjectTypes::BPART_BONE:
	case ObjectTypes::BPART_EGGLAYER:
	case ObjectTypes::BPART_GRIPPER:
	case ObjectTypes::BPART_MOUTH:
	case ObjectTypes::BPART_FATCELL:
	case ObjectTypes::BPART_NOSE:
		maxFoodAvailable = static_cast<BodyPart*>(pOther->userPointer_)->getFoodValue();
		break;
	case ObjectTypes::BPART_ZYGOTE:
		ERROR("Implement zygote eating - must check it's not owner - or have smell or something");
		break;
	default:
		ERROR("Mouth can't handle object type "<<(int)pOther->userObjectType_)
	};

	// compute food amount that will be transfered:
	float maxSwallow = min(maxFoodAvailable, maxSwallowRatio * maxFoodAvailable); // how much mass could we swallow if buffer allowed it?
	maxSwallow = min(maxSwallow, BodyConst::FoodSwallowSpeedDensity * size_ * lastDt_);	// mouth can only swallow so much in a given time
	float bufferAvail = bufferSize_ - usedBuffer_;
	float actualFoodAmountTransferred = min(bufferAvail, maxSwallow);
	usedBuffer_ += actualFoodAmountTransferred;
	assert(!std::isnan(usedBuffer_));

	// LOGLN("eat " << (int)pOther->userObjectType_ << " in amount: " << actualFoodAmountTransferred);

	// consume the food:
	switch (pOther->userObjectType_) {
	case ObjectTypes::FOOD_CHUNK: {
		static_cast<FoodChunk*>(pOther->userPointer_)->consume(actualFoodAmountTransferred);
		break;
	case ObjectTypes::BPART_BONE:
	case ObjectTypes::BPART_EGGLAYER:
	case ObjectTypes::BPART_GRIPPER:
	case ObjectTypes::BPART_MOUTH:
	case ObjectTypes::BPART_FATCELL:
	case ObjectTypes::BPART_NOSE:
		static_cast<BodyPart*>(pOther->userPointer_)->consumeFoodValue(actualFoodAmountTransferred);
		break;
	}
	default:
		ERROR("Mouth can't handle object type "<<(int)pOther->userObjectType_)
	};
}

void Mouth::update(float dt) {
	PERF_MARKER_FUNC;
	if (isDead())
		return;
	lastDt_ = dt;
	if (usedBuffer_ > 0) {
		// process food
		float amountProcessed = min(usedBuffer_, BodyConst::FoodProcessingSpeedDensity * size_ * dt);
		usedBuffer_ -= amountProcessed;
		context_.owner.onFoodProcessed(amountProcessed);
	}
}

float Mouth::getDensity(BodyCell const& cell) {
	return BodyConst::MouthDensity;
}

float Mouth::getRadius(BodyCell const& cell, float angle) {
	float aspectRatio = extractAspectRatio(cell);
	float length = sqrtf(cell.size() * aspectRatio);	// l = sqrt(s*a)
	float width = length / aspectRatio;			// w = l/a
	glm::vec2 p = getMouthAttachmentPoint(length, width, angle);
	return glm::length(p);
}

float Mouth::extractAspectRatio(BodyCell const& cell) {
	auto it = cell.mapAttributes_.find(GENE_ATTRIB_ASPECT_RATIO);
	auto aspVal = it != cell.mapAttributes_.end() ? it->second : CumulativeValue();
	aspVal.changeAbs(BodyConst::initialMouthAspectRatio);
	float aspectRatio = aspVal.clamp(
				BodyConst::MaxBodyPartAspectRatioInv,
				BodyConst::MaxBodyPartAspectRatio);
	return aspectRatio;
}
