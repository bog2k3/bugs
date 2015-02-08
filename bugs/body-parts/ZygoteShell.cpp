/*
 * ZygoteShell.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "ZygoteShell.h"
#include "BodyConst.h"
#include "../math/math2D.h"
#include "../math/box2glm.h"
#include "../utils/rand.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/RenderContext.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <Box2D/Box2D.h>

const glm::vec3 debug_color(0.5f, 0.5f, 0.5f);

ZygoteShell::ZygoteShell(glm::vec2 position, float mass)
	: BodyPart(BODY_PART_ZYGOTE_SHELL, std::make_shared<BodyPartInitializationData>())
	, mass_(mass)
{
	getInitializationData()->size.reset(mass*BodyConst::ZygoteDensityInv);
	getInitializationData()->density.reset(BodyConst::ZygoteDensity);
	getInitializationData()->angleOffset.reset(randf()*2*PI);

	physBody_.userObjectType_ = ObjectTypes::BPART_ZYGOTE;
	physBody_.userPointer_ = this;
	physBody_.categoryFlags_ = EventCategoryFlags::BODYPART;

	cacheInitializationData();
	physBody_.create(cachedProps_);
	commit();
	committed_ = true;
}

ZygoteShell::~ZygoteShell() {
}

void ZygoteShell::commit() {
	b2CircleShape shape;
	shape.m_p.Set(0, 0);
	shape.m_radius = sqrtf(size_*PI_INV);

	b2FixtureDef fixDef;
	fixDef.density = density_;
	fixDef.friction = 0.2f;
	fixDef.restitution = 0.3f;
	fixDef.shape = &shape;

	physBody_.b2Body_->CreateFixture(&fixDef);
}

void ZygoteShell::draw(RenderContext const& ctx) {
	glm::vec3 transform = getWorldTransformation();
	glm::vec2 pos = vec3xy(transform);
	ctx.shape->drawLine(pos,
			pos + glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), transform.z),
			0, debug_color);
}

void ZygoteShell::updateCachedDynamicPropsFromBody() {
	PhysicsProperties &props = cachedProps_;
	props.angle = physBody_.b2Body_->GetAngle();
	props.angularVelocity = physBody_.b2Body_->GetAngularVelocity();
	props.position = b2g(physBody_.b2Body_->GetPosition());
	props.velocity = b2g(physBody_.b2Body_->GetLinearVelocity());
}

float ZygoteShell::add(BodyPart* part, float angle) {
	children_[nChildren_++] = part;
	return angle;
}
