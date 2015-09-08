/*
 * Mouth.cpp
 *
 *  Created on: Jan 18, 2015
 *      Author: bog
 */

#include "Mouth.h"
#include "BodyConst.h"
#include "../entities/food/FoodChunk.h"
#include "../math/math2D.h"
#include "../math/box2glm.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include "../utils/log.h"
#include "../utils/UpdateList.h"
#include "../utils/assert.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(0.2f, 0.8f, 1.f);

#define DEBUG_DRAW_MOUTH

MouthInitializationData::MouthInitializationData()
	: aspectRatio(BodyConst::initialMouthAspectRatio) {
	size.reset(BodyConst::initialMouthSize);
}

void Mouth::cacheInitializationData() {
	BodyPart::cacheInitializationData();
	auto data = std::dynamic_pointer_cast<MouthInitializationData>(getInitializationData());
	float aspectRatio = data->aspectRatio.clamp(BodyConst::MaxBodyPartAspectRatioInv, BodyConst::MaxBodyPartAspectRatio);
	length_ = sqrtf(size_ * aspectRatio);	// l = sqrt(s*a)
	width_ = length_ / aspectRatio;			// w = l/a
}

Mouth::Mouth()
	: BodyPart(BodyPartType::MOUTH, std::make_shared<MouthInitializationData>())
	, length_(0)
	, width_(0)
	, bufferSize_(0)
	, usedBuffer_(0)
	, pJoint(nullptr)
{
	onCollisionEventHandle = physBody_.onCollision.add(std::bind(&Mouth::onCollision, this, std::placeholders::_1, std::placeholders::_2));
	physBody_.userObjectType_ = ObjectTypes::BPART_MOUTH;
	physBody_.userPointer_ = this;
	physBody_.categoryFlags_ = EventCategoryFlags::BODYPART;
	physBody_.collisionEventMask_ = EventCategoryFlags::FOOD;
}

Mouth::~Mouth() {
}

void Mouth::die() {
	if (getUpdateList())
		getUpdateList()->remove(this);
	physBody_.onCollision.remove(onCollisionEventHandle);
	physBody_.collisionEventMask_ = 0;
	physBody_.categoryFlags_ |= EventCategoryFlags::FOOD;
}

void Mouth::onAddedToParent() {
	assertDbg(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}

glm::vec2 Mouth::getChildAttachmentPoint(float relativeAngle) {
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(rayIntersectBox(length_, width_, relativeAngle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

void Mouth::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(
				&physBody_.b2Body_->GetFixtureList()[0]);
		physBody_.b2Body_->GetWorld()->DestroyJoint(pJoint);
		pJoint = nullptr;
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

	b2WeldJointDef jdef;
	jdef.bodyA = parent_->getBody().b2Body_;
	jdef.bodyB = physBody_.b2Body_;
	glm::vec2 parentAnchor = parent_->getChildAttachmentPoint(attachmentDirectionParent_);
	jdef.localAnchorA = g2b(parentAnchor);
	glm::vec2 childAnchor = getChildAttachmentPoint(angleOffset_);
	jdef.localAnchorB = g2b(childAnchor);
	pJoint = (b2WeldJoint*) physBody_.b2Body_->GetWorld()->CreateJoint(&jdef);
}

void Mouth::draw(RenderContext const& ctx) {
	if (committed_) {
		// nothing to draw, physics will draw for us
#ifdef DEBUG_DRAW_MOUTH
		float ratio = sqrt((getFoodValue() / density_) / size_);
		float widthLeft = width_ * ratio;
		float lengthLeft = length_ * ratio;
		glm::vec3 worldTransform = getWorldTransformation();
		ctx.shape->drawRectangleCentered(vec3xy(worldTransform), 0, glm::vec2(lengthLeft, widthLeft), worldTransform.z, glm::vec3(0.5f,0,1));
#endif
	} else {
		glm::vec3 worldTransform = getWorldTransformation();
		ctx.shape->drawRectangleCentered(vec3xy(worldTransform), 0, glm::vec2(length_, width_), worldTransform.z, debug_color);
		ctx.shape->drawLine(vec3xy(worldTransform),
				vec3xy(worldTransform) + glm::rotate(getChildAttachmentPoint(0), worldTransform.z),
				0, debug_color);
	}
}

void Mouth::onCollision(PhysicsBody* pOther, float impulseMagnitude) {
	if (impulseMagnitude > 1000) {
#warning "implement hurt"
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
		maxFoodAvailable = static_cast<FoodChunk*>(pOther->userPointer_)->getMassLeft();
		break;
	}
	case ObjectTypes::BPART_BONE:
	case ObjectTypes::BPART_EGGLAYER:
	case ObjectTypes::BPART_GRIPPER:
	case ObjectTypes::BPART_MOUTH:
	case ObjectTypes::BPART_TORSO:
		maxFoodAvailable = static_cast<BodyPart*>(pOther->userPointer_)->getFoodValue();
		break;
	//case ObjectTypes::BPART_ZYGOTE:
		//break;
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
	case ObjectTypes::BPART_TORSO:
		static_cast<BodyPart*>(pOther->userPointer_)->consumeFoodValue(actualFoodAmountTransferred);
		break;
	}
	default:
		ERROR("Mouth can't handle object type "<<(int)pOther->userObjectType_)
	};
}

void Mouth::update(float dt) {
	if (isDead())
		return;
	if (usedBuffer_ > 0)
		usedBuffer_ -= parent_->addFood(usedBuffer_);
}
