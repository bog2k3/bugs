/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../log.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

const glm::vec3 debug_color(0.f, 1.f, 0.f);

Bone::Bone(BodyPart* parent, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_BONE, props)
	, density_(1)
	, size_(1.e-4f, 0.7f)	// 1 sq cm, more wide than long
{
}

Bone::~Bone() {
}

void Bone::setDensity(float value) {
	assert(!committed_);
	density_ = value;
}
void Bone::setSize(glm::vec2 value) {
	assert(!committed_);
	size_ = value;
}
void Bone::commit() {
	assert(!committed_);
	// preprocess...
	// right now size_ contains x=surface_area and y=aspect_ratio=length/width
	// transform to width, length
	size_.x = sqrtf(size_.x/size_.y);
	size_.y *= size_.x;

	// create fixture:
	b2PolygonShape shape;
	shape.SetAsBox(size_.y * 0.5f, size_.x * 0.5f); // our x and y mean length and width, so are reversed (because length is parallel to OX axis)
	b2FixtureDef fixDef;
	fixDef.density = density_;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);
}
glm::vec2 Bone::getChildAttachmentPoint(float relativeAngle)
{
#error "this must take aspect ratio into account"
	// as if the angle is expressed for an aspect ratio of 1:1, and then the resulting point is stretched along the edge.

	// bring the angle between [-PI, +PI]
	relativeAngle = limitAngle(relativeAngle, PI);

	float hw = sqrtf(size_.x/size_.y) * 0.5f; // half width
	float hl = size_.y * hw; // half length

	float ac1 = atanf(hw/hl);
	if (relativeAngle >= 0) {
		if (relativeAngle <= ac1) {
			// front edge, left side
			return glm::vec2(hl, tanf(relativeAngle) * hl);
		} else if (relativeAngle <= PI - ac1) {
			// left edge
			if (eqEps(relativeAngle, PI*0.5f)) // treat singularity for tan at PI/2
				return glm::vec2(0, -hw);
			return glm::vec2(hw / tanf(relativeAngle), hw);
		} else {
			// back edge, left side
			return glm::vec2(-hl, -tanf(relativeAngle) * hl);
		}
	} else /* relativeAngle < 0 */ {
		if (relativeAngle >= -ac1) {
			// front edge, right side
			return glm::vec2(hl, tanf(relativeAngle) * hl);
		} else if (relativeAngle >= -PI + ac1) {
			// right edge
			if (eqEps(relativeAngle, -PI*0.5f)) // treat singularity for tan at -PI/2
				return glm::vec2(0, -hw);
			return glm::vec2(-hw / tanf(relativeAngle), hw);
		} else {
			// back edge, right side
			return glm::vec2(-hl, -tanf(relativeAngle) * hl);
		}
	}
}

void Bone::draw(ObjectRenderContext* ctx) {
	if (committed_) {
		// nothing to draw, physics will draw for us
	} else {
		glm::vec3 worldTransform = getWorldTransformation();
		float w = sqrtf(size_.x/size_.y);
		float l = size_.y * w;
		ctx->shape->drawRectangle(vec3xy(worldTransform), 0,
				glm::vec2(l, w), worldTransform.z, debug_color);
		ctx->shape->drawLine(
				vec3xy(worldTransform),
				vec3xy(worldTransform) + glm::rotate(getChildAttachmentPoint(0), worldTransform.z),
				0,
				debug_color);
	}
}
