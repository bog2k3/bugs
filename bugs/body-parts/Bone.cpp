/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include "BodyConst.h"
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
	cell.mapAttributes_[GENE_ATTRIB_ASPECT_RATIO].changeAbs(BodyConst::initialBoneAspectRatio);
	float aspectRatio = cell.mapAttributes_[GENE_ATTRIB_ASPECT_RATIO].clamp(
				BodyConst::MaxBodyPartAspectRatioInv,
				BodyConst::MaxBodyPartAspectRatio);
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
	auto densValue = cell.mapAttributes_[GENE_ATTRIB_GENERIC1];
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
glm::vec2 Bone::getAttachmentPoint(float relativeAngle)
{
	glm::vec2 ret(rayIntersectBox(length_, width_, relativeAngle));
	return ret;
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
