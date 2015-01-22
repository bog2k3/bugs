/*
 * Torso.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "Torso.h"
#include "BodyConst.h"
#include "../math/math2D.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/RenderContext.h"
#include "../utils/UpdateList.h"
#include "../utils/log.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(1.f, 1.f, 0.f);

Torso::Torso(BodyPart* parent)
	: BodyPart(parent, BODY_PART_TORSO, std::make_shared<BodyPartInitializationData>())
	, size_(0.f)
	, fatMass_(0.1)
	, lastCommittedTotalSizeInv_(0)
	, frameUsedEnergy_(0)
{
	physBody_.userObjectType_ = ObjectTypes::BPART_TORSO;
	physBody_.userPointer_ = this;
	physBody_.categoryFlags_ = CategoryFlags::BODYPART;

	getUpdateList()->add(this);
}

Torso::~Torso() {
}

void Torso::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
	}

	size_ = getInitializationData()->size;
	float fatSize = fatMass_ * BodyConst::FatDensityInv;
	lastCommittedTotalSizeInv_ = 1.f / (size_ + fatSize);

	float density = (size_ * BodyConst::TorsoDensity + fatMass_) / (size_ + fatMass_ * BodyConst::FatDensityInv);

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
}

void Torso::draw(RenderContext const& ctx) {
	if (committed_) {
		// nothing, physics draws
#ifdef DEBUG_DRAW_TORSO
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
#endif
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(getInitializationData()->size/PI), 0, 12, debug_color);
		ctx.shape->drawCircle(pos, sqrtf((getInitializationData()->size+fatMass_*BodyConst::FatDensityInv)/PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(getInitializationData()->size/PI), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 Torso::getChildAttachmentPoint(float relativeAngle) const
{
	float size = getInitializationData()->size + fatMass_*BodyConst::FatDensityInv;
	return glm::rotate(glm::vec2(sqrtf(size * PI_INV), 0), relativeAngle);
}

void Torso::setInitialFatMass(float fat) {
	assert(!committed_);
	fatMass_ = fat;
}

void Torso::consumeEnergy(float amount) {
	frameUsedEnergy_ += amount;

#warning "optimization: have a ready-to-use energy buffer instead of using fat all the time. use fat to replenish buffer. food replenishes buffer too."
}

void Torso::update(float dt) {
	fatMass_ -= frameUsedEnergy_ * BodyConst::FatEnergyDensityInv;
	float crtSize = fatMass_ * BodyConst::FatDensityInv + size_;
	if (crtSize * lastCommittedTotalSizeInv_ > BodyConst::SizeThresholdToCommit
			|| crtSize * lastCommittedTotalSizeInv_ < BodyConst::SizeThresholdToCommit_inv) {
		commit();
		reattachChildren();
	}
	frameUsedEnergy_ = 0;
}

void Torso::die() {
	commit();
}
