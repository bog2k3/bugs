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

Torso::Torso(BodyPart* parent)
	: BodyPart(parent, BODY_PART_TORSO, std::make_shared<TorsoInitializationData>())
	, torsoInitialData_(std::static_pointer_cast<TorsoInitializationData>(getInitializationData()))
{
	std::shared_ptr<TorsoInitializationData> initData = torsoInitialData_.lock();
	registerAttribute(GENE_ATTRIB_DENSITY, initData->density);
}

Torso::~Torso() {
	// delete fixture
}

void Torso::commit() {
	assert(!committed_);
	std::shared_ptr<TorsoInitializationData> initData = torsoInitialData_.lock();
	// create fixture....
	b2CircleShape shape;
	shape.m_p.Set(0, 0);
	shape.m_radius = sqrtf(initData->size/PI);

	b2FixtureDef fixDef;
	fixDef.density = initData->density;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);
}

void Torso::draw(RenderContext& ctx) {
	if (committed_) {
		// nothing, physics draws
	} else {
		/*glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos, pos + glm::rotate(glm::vec2(sqrtf(size_/PI), 0), transform.z), 0, debug_color);
		*/
	}
}

glm::vec2 Torso::getChildAttachmentPoint(float relativeAngle) const
{
	assert(!committed_);
	std::shared_ptr<TorsoInitializationData> initData = torsoInitialData_.lock();
	return glm::rotate(glm::vec2(sqrtf(initData->size * PI_INV), 0), relativeAngle);
}
