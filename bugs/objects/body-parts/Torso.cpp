/*
 * Torso.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "Torso.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(1.f, 1.f, 0.f);

Torso::Torso(BodyPart* parent, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_TORSO, props)
	, size_(0.5e-3f) // 10 sq cm
	, density_(1.f)
{
	registerAttribute(GENE_ATTRIB_SIZE, size_);
	registerAttribute(GENE_ATTRIB_DENSITY, density_);
}

Torso::~Torso() {
	// delete fixture
}

void Torso::commit() {
	assert(!committed_);
	// create fixture....
	b2CircleShape shape;
	shape.m_p.Set(0, 0);
	shape.m_radius = sqrtf(size_/PI);

	b2FixtureDef fixDef;
	fixDef.density = density_;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);
}

void Torso::draw(RenderContext* ctx) {
	if (committed_) {
		// nothing, physics draws
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx->shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
		ctx->shape->drawLine(pos, pos + glm::rotate(glm::vec2(sqrtf(size_/PI), 0), transform.z), 0, debug_color);
	}
}

glm::vec2 Torso::getChildAttachmentPoint(float relativeAngle)
{
	return glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle);
}
