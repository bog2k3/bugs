/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include "BodyConst.h"
#include "../math/math2D.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/RenderContext.h"
#include "../utils/log.h"
#include "../utils/assert.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

const glm::vec3 debug_color(0.f, 1.f, 0.f);

#define DEBUG_DRAW_BONE

BoneInitializationData::BoneInitializationData()
	: aspectRatio(BodyConst::initialBoneAspectRatio) {
	density.reset(BodyConst::initialBoneDensity);
}

void Bone::cacheInitializationData() {
	BodyPart::cacheInitializationData();
	auto initData = std::dynamic_pointer_cast<BoneInitializationData>(getInitializationData());
	float aspectRatio = initData->aspectRatio.clamp(BodyConst::MaxBodyPartAspectRatioInv, BodyConst::MaxBodyPartAspectRatio);
	length_ = sqrtf(size_ * aspectRatio);	// l = sqrt(s*a)
	width_ = length_ / aspectRatio;			// w = l/a
}

Bone::Bone()
	: BodyPart(BODY_PART_BONE, std::make_shared<BoneInitializationData>())
	, length_(0)
	, width_(0)
{
	auto initData = std::dynamic_pointer_cast<BoneInitializationData>(getInitializationData());
	registerAttribute(GENE_ATTRIB_ASPECT_RATIO, initData->aspectRatio);
	registerAttribute(GENE_ATTRIB_DENSITY, initData->density);

	physBody_.userObjectType_ = ObjectTypes::BPART_BONE;
	physBody_.userPointer_ = this;
	physBody_.categoryFlags_ = EventCategoryFlags::BODYPART;
}

Bone::~Bone() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
	}
}

void Bone::die() {
	physBody_.categoryFlags_ |= EventCategoryFlags::FOOD;
}

void Bone::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
	}

	// create fixture:
	b2PolygonShape shape;
	shape.SetAsBox(length_ * 0.5f, width_ * 0.5f); // our x and y mean length and width (because length is parallel to OX axis)
	b2FixtureDef fixDef;
	fixDef.density = density_;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);
}
glm::vec2 Bone::getChildAttachmentPoint(float relativeAngle)
{
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(rayIntersectBox(length_, width_, relativeAngle));
	return ret;
}

void Bone::draw(RenderContext const& ctx) {
	if (committed_) {
		// nothing to draw, physics will draw for us
#ifdef DEBUG_DRAW_BONE
		if (isDead()) {
			float ratio = sqrt((getFoodValue() / density_) / size_);
			float widthLeft = width_ * ratio;
			float lengthLeft = length_ * ratio;
			glm::vec3 worldTransform = getWorldTransformation();
			ctx.shape->drawRectangle(vec3xy(worldTransform), 0,
				glm::vec2(lengthLeft, widthLeft), worldTransform.z, glm::vec3(0.5f,0,1));
		}
#endif
	} else {
		glm::vec3 worldTransform = getWorldTransformation();
		ctx.shape->drawRectangle(vec3xy(worldTransform), 0,
				glm::vec2(length_, width_), worldTransform.z, debug_color);
		ctx.shape->drawLine(
				vec3xy(worldTransform),
				vec3xy(worldTransform) + glm::rotate(getChildAttachmentPoint(0), worldTransform.z),
				0,
				debug_color);
	}
}

float Bone::getAspectRatio() {
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	return length_ / width_;
}

