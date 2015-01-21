/*
 * Mouth.cpp
 *
 *  Created on: Jan 18, 2015
 *      Author: bog
 */

#include "Mouth.h"
#include "BodyConst.h"
#include "../../math/math2D.h"
#include "../../math/box2glm.h"
#include "../../renderOpenGL/RenderContext.h"
#include "../../renderOpenGL/Shape2D.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(0.2f, 0.8f, 1.f);

Mouth::Mouth(BodyPart* parent)
	: BodyPart(parent, BODY_PART_MOUTH, std::make_shared<BodyPartInitializationData>())
	, linearSize_(0)
	, pJoint(nullptr)
{
	getInitializationData()->size.reset(BodyConst::initialMouthSize);
}
Mouth::~Mouth() {
}

glm::vec2 Mouth::getChildAttachmentPoint(float relativeAngle) const {
	if (!committed_) {
		float size = getInitializationData()->size;
		float width = sqrtf(size / BodyConst::MouthAspectRatio);
		float length = BodyConst::MouthAspectRatio * width;
		return rayIntersectBox(width, length, relativeAngle) * (-0.7f);
	}
	return glm::vec2(0);
}

void Mouth::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
		physBody_.b2Body_->GetWorld()->DestroyJoint(pJoint);
		pJoint = nullptr;
	}

	float size = getInitializationData()->size;
	float width = sqrtf(size / BodyConst::MouthAspectRatio);
	float length = BodyConst::MouthAspectRatio * width;

	// create fixture:
	b2PolygonShape shape;
	shape.SetAsBox(length * 0.5f, width * 0.5f); // our x and y mean length and width, so are reversed (because length is parallel to OX axis)
	b2FixtureDef fixDef;
	fixDef.density = BodyConst::MouthDensity;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);

	b2WeldJointDef jdef;
	jdef.bodyA = parent_->getBody().b2Body_;
	jdef.bodyB = physBody_.b2Body_;
	glm::vec2 parentAnchor = parent_->getChildAttachmentPoint(getInitializationData()->attachmentDirectionParent);
	jdef.localAnchorA = g2b(parentAnchor);
	glm::vec2 childAnchor = getChildAttachmentPoint(PI - getInitializationData()->angleOffset);
	jdef.localAnchorB = g2b(childAnchor);
	pJoint = (b2WeldJoint*)physBody_.b2Body_->GetWorld()->CreateJoint(&jdef);
}

void Mouth::draw(RenderContext& ctx) {
	if (committed_) {
		// nothing to draw, physics will draw for us
	} else {
		glm::vec3 worldTransform = getWorldTransformation();
		float size = getInitializationData()->size;
		float width = sqrtf(size / BodyConst::MouthAspectRatio);
		float length = BodyConst::MouthAspectRatio * width;
		ctx.shape->drawRectangle(vec3xy(worldTransform), 0,
				glm::vec2(length, width), worldTransform.z, debug_color);
		ctx.shape->drawLine(
				vec3xy(worldTransform),
				vec3xy(worldTransform) + glm::rotate(getChildAttachmentPoint(0), worldTransform.z),
				0,
				debug_color);
	}
}
