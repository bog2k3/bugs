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
#include "../ObjectTypesAndFlags.h"

#include <boglfw/World.h>
#include <boglfw/math/math3D.h>
#include <boglfw/math/box2glm.h>
#include <boglfw/renderOpenGL/RenderContext.h>
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
	cell.mapAttributes_[GENE_ATTRIB_ASPECT_RATIO].changeAbs(BodyConst::initialMouthAspectRatio);
	float aspectRatio = cell.mapAttributes_[GENE_ATTRIB_ASPECT_RATIO].clamp(
				BodyConst::MaxBodyPartAspectRatioInv,
				BodyConst::MaxBodyPartAspectRatio);
	length_ = sqrtf(size_ * aspectRatio);	// l = sqrt(s*a)
	width_ = length_ / aspectRatio;			// w = l/a

	onCollisionEventHandle_ = physBody_.onCollision.add(std::bind(&Mouth::onCollision, this, std::placeholders::_1, std::placeholders::_2));
	physBody_.userObjectType_ = ObjectTypes::BPART_MOUTH;
	physBody_.userPointer_ = this;
	physBody_.collisionEventMask_ = EventCategoryFlags::FOOD;

	context.updateList.add(this);
}

Mouth::~Mouth() {
}

void Mouth::die() {
	context_.updateList.remove(this);
	physBody_.onCollision.remove(onCollisionEventHandle_);
	physBody_.collisionEventMask_ = 0;
}

glm::vec2 Mouth::getAttachmentPoint(float relativeAngle) {
	glm::vec2 ret(rayIntersectBox(length_, width_, relativeAngle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

void Mouth::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (physBody_.b2Body_->GetFixtureList()) {
		physBody_.b2Body_->DestroyFixture(
				&physBody_.b2Body_->GetFixtureList()[0]);
//		physBody_.b2Body_->GetWorld()->DestroyJoint(pJoint);
//		pJoint = nullptr;
	}

	bufferSize_ = size_ * BodyConst::MouthBufferDensity;

	// create fixture:
	b2PolygonShape shape;
	shape.SetAsBox(length_ * 0.5f, width_ * 0.5f); // our x and y mean length and width, so are reversed (because length is parallel to OX axis)
	b2FixtureDef fixDef;
	fixDef.density = BodyConst::MouthDensity;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);

	/*b2WeldJointDef jdef;
	jdef.bodyA = parent_->getBody().b2Body_;
	jdef.bodyB = physBody_.b2Body_;
	glm::vec2 parentAnchor = parent_->getAttachmentPoint(attachmentDirectionParent_);
	jdef.localAnchorA = g2b(parentAnchor);
	glm::vec2 childAnchor = getAttachmentPoint(PI - localRotation_);
	jdef.localAnchorB = g2b(childAnchor);
	pJoint = (b2WeldJoint*) physBody_.b2Body_->GetWorld()->CreateJoint(&jdef);*/
}

void Mouth::draw(RenderContext const& ctx) {
#ifdef DEBUG_DRAW_MOUTH
	float ratio = sqrt((getFoodValue() / density_) / size_);
	float widthLeft = width_ * ratio;
	float lengthLeft = length_ * ratio;
	glm::vec3 worldTransform = getWorldTransformation();
	Shape3D::get()->drawRectangleXOYCentered(glm::vec3(vec3xy(worldTransform), 0), glm::vec2(lengthLeft, widthLeft), worldTransform.z, glm::vec3(0.5f,0,1));
#endif
}

void Mouth::onCollision(PhysicsBody* pOther, float impulseMagnitude) {
	if (impulseMagnitude > 1000) {
		//TODO implement hurt
		throw std::runtime_error("Implement this!");
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
	float bufferAvail = bufferSize_ - usedBuffer_;
	float actualFoodAmountTransferred = min(bufferAvail, maxSwallow);
	usedBuffer_ += actualFoodAmountTransferred;

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
	/*if (usedBuffer_ > 0)
		usedBuffer_ -= parent_->addFood(usedBuffer_);*/
	// TODO food addition
	throw std::runtime_error("Implement this!");
}

float Mouth::getDensity(BodyCell const& cell) {
	return BodyConst::MouthDensity;
}
