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

MouthInitializationData::MouthInitializationData()
	: aspectRatio(BodyConst::MouthAspectRatio) {
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
	: BodyPart(BODY_PART_MOUTH, std::make_shared<MouthInitializationData>())
	, length_(0)
	, width_(0)
	, bufferSize_(0)
	, usedBuffer_(0)
	, pJoint(nullptr)
{
	physBody_.onCollision.add(std::bind(&Mouth::onCollision, this, std::placeholders::_1, std::placeholders::_2));
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
}

void Mouth::onAddedToParent() {
	assert(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}

glm::vec2 Mouth::getChildAttachmentPoint(float relativeAngle) {
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(rayIntersectBox(length_, width_, relativeAngle));
	assert(!std::isnan(ret.x) && !std::isnan(ret.y));
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
	} else {
		glm::vec3 worldTransform = getWorldTransformation();
		ctx.shape->drawRectangle(vec3xy(worldTransform), 0, glm::vec2(length_, width_), worldTransform.z, debug_color);
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
	switch (pOther->userObjectType_) {
	case ObjectTypes::FOOD_CHUNK: {
		FoodChunk* pChunk = static_cast<FoodChunk*>(pOther->userPointer_);
		// how much mass could we swallow if buffer allowed it?
		float maxSwallow = min(pChunk->getMassLeft(), maxSwallowRatio * pChunk->getInitialMass());
		float bufferAvail = bufferSize_ - usedBuffer_;
		float actualFoodAmountTransferred = min(bufferAvail, maxSwallow);
		usedBuffer_ += actualFoodAmountTransferred;
		pChunk->consume(actualFoodAmountTransferred);
		break;
	}
	default:
		ERROR("Mouth can't handle object type "<<(int)pOther->userObjectType_)
	};
}

void Mouth::update(float dt) {
	if (usedBuffer_ > 0)
		usedBuffer_ -= parent_->addFood(usedBuffer_);
}
