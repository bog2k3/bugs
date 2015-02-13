/*
 * EggLayer.cpp
 *
 *  Created on: Feb 10, 2015
 *      Author: bogdan
 */

#include "EggLayer.h"
#include "../math/box2glm.h"
#include "../math/math2D.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/RenderContext.h"
#include "../utils/UpdateList.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Box2D.h>

static const glm::vec3 debug_color(0.2f, 0.7f, 1.0f);

EggLayer::EggLayer()
	: BodyPart(BODY_PART_EGGLAYER, std::make_shared<BodyPartInitializationData>())
{
}

EggLayer::~EggLayer() {
	if (getUpdateList())
		getUpdateList()->remove(this);
}

void EggLayer::draw(RenderContext const& ctx) {
	if (committed_) {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		float r_2 = sqrtf(size_*PI_INV) * 0.5f;
		ctx.shape->drawLine(
			pos - glm::rotate(glm::vec2(r_2, 0), transform.z),
			pos + glm::rotate(glm::vec2(r_2, 0), transform.z),
			0, debug_color);
		ctx.shape->drawLine(
			pos - glm::rotate(glm::vec2(r_2, 0), transform.z+PI*0.5f),
			pos + glm::rotate(glm::vec2(r_2, 0), transform.z+PI*0.5f),
			0, debug_color);
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_*PI_INV), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(size_*PI_INV), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 EggLayer::getChildAttachmentPoint(float relativeAngle) {
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle));
	assert(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

void EggLayer::update(float dt){

}

void EggLayer::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
	};

	b2CircleShape shape;
	shape.m_radius = sqrtf(size_ * PI_INV);
	b2FixtureDef fdef;
	fdef.density = density_;
	fdef.friction = 0.3f;
	fdef.restitution = 0.2f;
	fdef.shape = &shape;
	physBody_.b2Body_->CreateFixture(&fdef);
}

void EggLayer::onAddedToParent() {
	assert(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}
