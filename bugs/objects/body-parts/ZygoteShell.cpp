/*
 * ZygoteShell.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "ZygoteShell.h"
#include "BodyConst.h"
#include "../../math/math2D.h"
#include "../../math/box2glm.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <Box2D/Box2D.h>

const glm::vec3 debug_color(0.5f, 0.5f, 0.5f);

ZygoteShell::ZygoteShell(float size)
	: BodyPart(nullptr, BODY_PART_ZYGOTE_SHELL, std::make_shared<BodyPartInitializationData>())
	, mass_(size*BodyConst::ZygoteDensity)
{
	getInitializationData()->size.reset(size);
	getInitializationData()->density.reset(BodyConst::ZygoteDensity);

	commit_tree();
}

ZygoteShell::~ZygoteShell() {
}

void ZygoteShell::commit() {
	b2CircleShape shape;
	shape.m_p.Set(0, 0);
	shape.m_radius = sqrtf(getInitializationData()->size/PI);

	b2FixtureDef fixDef;
	fixDef.density = getInitializationData()->density;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	body_->CreateFixture(&fixDef);
}

void ZygoteShell::draw(RenderContext& ctx) {
	glm::vec3 transform = getWorldTransformation();
	glm::vec2 pos = vec3xy(transform);
	ctx.shape->drawLine(pos,
			pos + glm::rotate(glm::vec2(sqrtf(getInitializationData()->size / PI), 0), transform.z),
			0, debug_color);
}

void ZygoteShell::updateCachedDynamicPropsFromBody() {
	PhysicsProperties &props = getInitializationData()->cachedProps;
	props.angle = body_->GetAngle();
	props.angularVelocity = body_->GetAngularVelocity();
	props.position = b2g(body_->GetPosition());
	props.velocity = b2g(body_->GetLinearVelocity());
}
