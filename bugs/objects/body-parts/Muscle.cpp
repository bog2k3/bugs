/*
 * Muscle.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#include "Muscle.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include <glm/vec3.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <math.h>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(1.f,0.2f, 0.8f);

Muscle::Muscle(BodyPart* parent, PhysicsProperties props)
: BodyPart(parent, BODY_PART_MUSCLE, props)
, size_(0.5e-4f)
, aspectRatio_(0.7f)
, insertionOffset_(0.5f)
{
	// we need this for debug draw, since muscle doesn't create fixture, nor body
	keepInitializationData_ = true;
	dontCommit_ = true;
}

Muscle::~Muscle() {
}

glm::vec2 Muscle::getChildAttachmentPoint(float relativeAngle)
{
	// this also takes aspect ratio into account as if the angle is expressed
	// for an aspect ratio of 1:1, and then the resulting point is stretched along the edge.

	// bring the angle between [-PI, +PI]
	relativeAngle = limitAngle(relativeAngle, 7*PI/4);
	float hw = sqrtf(size_/aspectRatio_) * 0.5f; // half width
	float hl = aspectRatio_ * hw; // half length
	if (relativeAngle < PI/4) {
		// front edge
		return glm::vec2(hl, sinf(relativeAngle) / sinf(PI/4) * hw);
	} else if (relativeAngle < 3*PI/4 || relativeAngle > 5*PI/4) {
		// left or right edge
		return glm::vec2(cosf(relativeAngle) / cosf(PI/4) * hl, relativeAngle < PI ? hw : -hw);
	} else {
		// back edge
		return glm::vec2(-hl, sinf(relativeAngle) / sinf(PI/4) * hw);
	}
}

void Muscle::draw(RenderContext& ctx) {
	initialData_->position = getFinalPrecommitPosition();
	glm::vec3 worldTransform = getWorldTransformation();
	float w = sqrtf(size_/aspectRatio_);
	float l = aspectRatio_ * w;
	ctx.shape->drawRectangle(vec3xy(worldTransform), 0,
			glm::vec2(l, w), worldTransform.z, debug_color);
	ctx.shape->drawLine(
			vec3xy(worldTransform),
			vec3xy(worldTransform) + glm::rotate(getChildAttachmentPoint(0), worldTransform.z),
			0,
			debug_color);
}
