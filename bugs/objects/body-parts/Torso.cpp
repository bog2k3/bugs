/*
 * Torso.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "Torso.h"
#include "BodyConst.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(1.f, 1.f, 0.f);

Torso::Torso(BodyPart* parent)
	: BodyPart(parent, BODY_PART_TORSO, std::make_shared<BodyPartInitializationData>())
	, size_(0.f)
	, fatMass_(0.1)
{
}

Torso::~Torso() {
	// delete fixture
}

void Torso::commit() {
	if (committed_) {
		body_->DestroyFixture(&body_->GetFixtureList()[0]);
	}

	size_ = getInitializationData()->size;
	float fatSize = fatMass_ / BodyConst::FatDensity;

	float density = (size_*BodyConst::TorsoDensity + fatMass_) / (size_ + fatMass_/BodyConst::FatDensity);

	// create fixture....
	b2CircleShape shape;
	shape.m_p.Set(0, 0);
	shape.m_radius = sqrtf((size_+fatSize)/PI);

	b2FixtureDef fixDef;
	fixDef.density = density;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);
}

void Torso::draw(RenderContext& ctx) {
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
		ctx.shape->drawCircle(pos, sqrtf((getInitializationData()->size+fatMass_/BodyConst::FatDensity)/PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(getInitializationData()->size/PI), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 Torso::getChildAttachmentPoint(float relativeAngle) const
{
	float size = getInitializationData()->size + fatMass_/BodyConst::FatDensity;
	return glm::rotate(glm::vec2(sqrtf(size * PI_INV), 0), relativeAngle);
}

void Torso::setFatMass(float fat) {
	assert(!committed_);
	fatMass_ = fat;
}
