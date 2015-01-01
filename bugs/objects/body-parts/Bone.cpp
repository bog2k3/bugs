/*
 * Bone.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Bone.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include "../../log.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

const glm::vec3 debug_color(0.f, 1.f, 0.f);

Bone::Bone(BodyPart* parent)
	: BodyPart(parent, BODY_PART_BONE, std::make_shared<BoneInitializationData>())
	, boneInitialData_(std::static_pointer_cast<BoneInitializationData>(getInitializationData()))
{
	std::shared_ptr<BoneInitializationData> initData = boneInitialData_.lock();
	registerAttribute(GENE_ATTRIB_ASPECT_RATIO, initData->aspectRatio);
	registerAttribute(GENE_ATTRIB_DENSITY, initData->density);
}

Bone::~Bone() {
}

void Bone::commit() {
	assert(!committed_);

	std::shared_ptr<BoneInitializationData> initData = boneInitialData_.lock();

	glm::vec2 boxSize((float)initData->size, (float)initData->aspectRatio);
	boxSize.x = sqrtf(boxSize.x/boxSize.y);
	boxSize.y *= boxSize.x;

	// create fixture:
	b2PolygonShape shape;
	shape.SetAsBox(boxSize.y * 0.5f, boxSize.x * 0.5f); // our x and y mean length and width, so are reversed (because length is parallel to OX axis)
	b2FixtureDef fixDef;
	fixDef.density = initData->density;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);
}
glm::vec2 Bone::getChildAttachmentPoint(float relativeAngle) const
{
	// this also takes aspect ratio into account as if the angle is expressed
	// for an aspect ratio of 1:1, and then the resulting point is stretched along the edge.

	std::shared_ptr<BoneInitializationData> initData = boneInitialData_.lock();

	// bring the angle between [-PI, +PI]
	relativeAngle = limitAngle(relativeAngle, 7*PI/4);
	float hw = sqrtf(initData->size / initData->aspectRatio) * 0.5f; // half width
	float hl = initData->aspectRatio * hw; // half length
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

void Bone::draw(RenderContext& ctx) {
	if (committed_) {
		// nothing to draw, physics will draw for us
	} else {
		/*initialData_->position = getFinalPrecommitPosition();
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
		*/
	}
}
