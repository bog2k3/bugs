/*
 * Torso.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "Torso.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(1.f, 1.f, 0.f);

Torso::Torso(BodyPart* parent, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_TORSO, props)
	, size_(1.e-3f) // 10 sq cm
	, density_(1.f)
{
}

Torso::~Torso() {
	// delete fixture
}

void Torso::commit() {
	assert(!committed_);
	// create fixture....
}

void Torso::setSize(float val) {
	assert(!committed_);
	size_ = val;
}

void Torso::setDensity(float val) {
	assert(!committed_);
	density_ = val;
}

void Torso::draw(ObjectRenderContext* ctx) {
	if (committed_) {
		// nothing, physics draws
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx->shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
		ctx->shape->drawLine(pos, pos + glm::rotate(glm::vec2(sqrtf(size_/PI), 0), transform.z), 0, debug_color);
	}
}

glm::vec2 Torso::getRelativeAttachmentPoint(float relativeAngle)
{
	assert(!committed_);
	return glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle);
}
