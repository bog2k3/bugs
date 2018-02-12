/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include "BodyConst.h"
#include "BodyCell.h"
#include "../ObjectTypesAndFlags.h"

#include <boglfw/World.h>
#include <boglfw/math/math3D.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/assert.h>

#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

#define DEBUG_DRAW_BONE

Bone::Bone(BodyPartContext const& context, BodyCell& cell)
	: BodyPart(BodyPartType::BONE, context, cell)
	, length_(0)
	, width_(0)
{
	float aspectRatio = extractAspectRatio(cell);
	length_ = sqrtf(size_ * aspectRatio);	// l = sqrt(s*a)
	width_ = length_ / aspectRatio;			// w = l/a

	physBody_.userObjectType_ = ObjectTypes::BPART_BONE;
	physBody_.userPointer_ = this;
}

Bone::~Bone() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
}

float Bone::getDensity(BodyCell const& cell) {
	auto it = cell.mapAttributes_.find(GENE_ATTRIB_GENERIC1);
	auto densValue = it != cell.mapAttributes_.end() ? it->second : CumulativeValue();
	densValue.changeAbs(BodyConst::initialBoneDensity);
	return densValue.clamp(BodyConst::MinBodyPartDensity, BodyConst::MaxBodyPartDensity);
}

void Bone::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	if (physBody_.b2Body_->GetFixtureList()) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
	}

	// create fixture:
	b2PolygonShape shape;
	shape.SetAsBox(length_ * 0.5f, width_ * 0.5f); // our x and y mean length and width (because length is parallel to OX axis)
	b2FixtureDef fixDef;
	fixDef.density = density_;
	fixDef.friction = 0.2f;		// TODO replace with BodyConst::constant
	fixDef.restitution = 0.3f;	// same
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);
}

static glm::vec2 getBoneAttachmentPoint(float length, float width, float angle) {
	glm::vec2 ret(rayIntersectBox(length, width, angle));
	return ret;
}

glm::vec2 Bone::getAttachmentPoint(float relativeAngle)
{
	return getBoneAttachmentPoint(length_, width_, relativeAngle);
}

void Bone::draw(RenderContext const& ctx) {
#ifdef DEBUG_DRAW_BONE
	if (isDead()) {
		float ratio = sqrt((getFoodValue() / density_) / size_);
		float widthLeft = width_ * ratio;
		float lengthLeft = length_ * ratio;
		glm::vec3 worldTransform = getWorldTransformation();
		Shape3D::get()->drawRectangleXOYCentered(vec3xy(worldTransform),
			glm::vec2(lengthLeft, widthLeft), worldTransform.z, glm::vec3(0.5, 0, 1));
	}
#endif
}

float Bone::getRadius(BodyCell const& cell, float angle) {
	float aspectRatio = extractAspectRatio(cell);
	float length = sqrtf(cell.size() * aspectRatio);	// l = sqrt(s*a)
	float width = length / aspectRatio;			// w = l/a
	glm::vec2 p = getBoneAttachmentPoint(length, width, angle);
	return glm::length(p);
}

float Bone::extractAspectRatio(BodyCell const& cell) {
	auto it = cell.mapAttributes_.find(GENE_ATTRIB_ASPECT_RATIO);
	auto aspVal = it != cell.mapAttributes_.end() ? it->second : CumulativeValue();
	aspVal.changeAbs(BodyConst::initialBoneAspectRatio);
	float aspectRatio = aspVal.clamp(
				BodyConst::MaxBodyPartAspectRatioInv,
				BodyConst::MaxBodyPartAspectRatio);
	return aspectRatio;
}
