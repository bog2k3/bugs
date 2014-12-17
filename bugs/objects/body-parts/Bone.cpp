/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include "../../math/math2D.h"
#include <Box2D/Box2D.h>

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
	shape.SetAsBox(size_.x * 0.5f, size_.y * 0.5f);
	b2FixtureDef fixDef;
	fixDef.density = density_;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);
}
glm::vec2 Bone::getRelativeAttachmentPoint(float relativeAngle)
{
	assert(!committed_);

	// bring the angle between [-PI, +PI]
	relativeAngle = limitAngle(relativeAngle, PI);

	float hw = sqrtf(size_.x/size_.y) * 0.5f; // half width
	float hh = size_.y * hw; // half height

	float ac1 = atanf(hw/hh);
	if (relativeAngle >= 0) {
		if (relativeAngle <= ac1) {
			// top edge, left side
			return glm::vec2(tanf(relativeAngle) * hh, hh);
		} else if (relativeAngle <= PI - ac1) {
			// left edge
			if (eqEps(relativeAngle, PI*0.5f)) // treat singularity for tan at PI/2
				return glm::vec2(-hw, 0);
			return glm::vec2(-hw, hw / tanf(relativeAngle));
		} else {
			// bottom edge, left side
			return glm::vec2(-tanf(relativeAngle) * hh, -hh);
		}
	} else /* relativeAngle < 0 */ {
		if (relativeAngle >= -ac1) {
			// top edge, right side
			return glm::vec2(tanf(relativeAngle) * hh, hh);
		} else if (relativeAngle >= -PI + ac1) {
			// right edge
			if (eqEps(relativeAngle, -PI*0.5f)) // treat singularity for tan at -PI/2
				return glm::vec2(hw, 0);
			return glm::vec2(hw, hw / tanf(-relativeAngle));
		} else {
			// bottom edge, right side
			return glm::vec2(-tanf(relativeAngle) * hh, -hh);
		}
	}
}
