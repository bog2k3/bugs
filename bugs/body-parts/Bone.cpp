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

BoneInitializationData::BoneInitializationData()
	: aspectRatio(BodyConst::initialBoneAspectRatio) {
	density.reset(BodyConst::initialBoneDensity);
}

void BoneInitializationData::sanitizeData() {
	BodyPartInitializationData::sanitizeData();
	if (aspectRatio > BodyConst::MaxBodyPartAspectRatio)
		aspectRatio.reset(BodyConst::MaxBodyPartAspectRatio);
	if (aspectRatio < BodyConst::MaxBodyPartAspectRatioInv)
		aspectRatio.reset(BodyConst::MaxBodyPartAspectRatioInv);
}

Bone::Bone(BodyPart* parent)
	: BodyPart(parent, BODY_PART_BONE, std::make_shared<BoneInitializationData>())
	, boneInitialData_(std::dynamic_pointer_cast<BoneInitializationData>(getInitializationData()))
	, size_(0)
	, density_(0)
{
	std::shared_ptr<BoneInitializationData> initData = boneInitialData_.lock();
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

void Bone::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
	}

	std::shared_ptr<BoneInitializationData> initData = boneInitialData_.lock();
	size_ = glm::vec2((float)initData->size, (float)initData->aspectRatio);
	size_.x = sqrtf(size_.x * size_.y);	// l = sqrt(s*a)
	size_.y = size_.x / size_.y;		// w = l/a
	density_ = initData->density;

	// create fixture:
	b2PolygonShape shape;
	shape.SetAsBox(size_.x * 0.5f, size_.y * 0.5f); // our x and y mean length and width, so are reversed (because length is parallel to OX axis)
	b2FixtureDef fixDef;
	fixDef.density = initData->density;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);
}
glm::vec2 Bone::getChildAttachmentPoint(float relativeAngle) const
{
	glm::vec2 size = size_;
	if (!committed_) {
		std::shared_ptr<BoneInitializationData> initData = boneInitialData_.lock();
		assertDbg(!std::isnan(initData->aspectRatio.get()) && initData->aspectRatio > 0);
		size.y = sqrtf(initData->size / initData->aspectRatio);
		size.x = initData->aspectRatio * size.y;
	}
	glm::vec2 ret(rayIntersectBox(size.y, size.x, relativeAngle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

void Bone::draw(RenderContext const& ctx) {
	if (committed_) {
		// nothing to draw, physics will draw for us
	} else {
		std::shared_ptr<BoneInitializationData> initData = boneInitialData_.lock();
		glm::vec3 worldTransform = getWorldTransformation();
		float w = sqrtf(initData->size / initData->aspectRatio);
		float l = initData->aspectRatio * w;
		ctx.shape->drawRectangle(vec3xy(worldTransform), 0,
				glm::vec2(l, w), worldTransform.z, debug_color);
		ctx.shape->drawLine(
				vec3xy(worldTransform),
				vec3xy(worldTransform) + glm::rotate(getChildAttachmentPoint(0), worldTransform.z),
				0,
				debug_color);
	}
}
