/*
 * Mouth.cpp
 *
 *  Created on: Jan 18, 2015
 *      Author: bog
 */

#include "Mouth.h"
#include "BodyConst.h"
#include "../../math/math2D.h"

Mouth::Mouth(BodyPart* parent)
	: BodyPart(parent, BODY_PART_MOUTH, std::make_shared<BodyPartInitializationData>())
	, linearSize_(0)
{
}
Mouth::~Mouth() {
	if (committed_) {
#warning "delete fixture"
		// ...
	}
}

glm::vec2 Mouth::getChildAttachmentPoint(float relativeAngle) const {
	if (!committed_) {
		float size = getInitializationData()->size;
		float width = sqrtf(size / BodyConst::MouthAspectRatio);
		float length = BodyConst::MouthAspectRatio * width;
		return rayIntersectBox(width, length, relativeAngle);
	}
	return glm::vec2(0);
}

void Mouth::draw(RenderContext& ctx) {

}

void Mouth::commit() {

}
