/*
 * ZygoteShell.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "ZygoteShell.h"
#include "BodyConst.h"
#include "../ObjectTypesAndFlags.h"

#include <boglfw/World.h>
#include <boglfw/math/math3D.h>
#include <boglfw/math/box2glm.h>
#include <boglfw/utils/rand.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/RenderContext.h>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <Box2D/Box2D.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

const glm::vec3 debug_color(0.5f, 0.5f, 0.5f);

ZygoteShell::ZygoteShell(glm::vec2 position, glm::vec2 velocity, float mass)
	: BodyPart(BodyPartType::ZYGOTE_SHELL, std::make_shared<BodyPartInitializationData>())
	, mass_(mass)
	, dead_(false)
{
	size_ = mass * BodyConst::ZygoteDensityInv;
	density_ = BodyConst::ZygoteDensity;
	localRotation_ = randf() * 2*PI;
#warning "these values above require some updating in the physBody"

	physBody_.userObjectType_ = ObjectTypes::BPART_ZYGOTE;
	physBody_.userPointer_ = this;

//	cacheInitializationData();
//	computeBodyPhysProps();
//	cachedProps_.position = position;
//	cachedProps_.velocity = velocity;

	World::getInstance()->queueDeferredAction([this]() {
		physBody_.create(cachedProps_);
		updateFixtures();
		committed_ = true;
	});
}

ZygoteShell::~ZygoteShell() {
}

void ZygoteShell::updateFixtures() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
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
	if (dead_) {
		Shape3D::get()->drawLine(
			{pos + glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), 3*PI/4 + transform.z), 0},
			{pos + glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), -PI/4 + transform.z), 0},
			debug_color);
		Shape3D::get()->drawLine(
			{pos + glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), PI/4 + transform.z), 0},
			{pos + glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), -3*PI/4 + transform.z), 0},
			debug_color);
	} else {
		Shape3D::get()->drawLine({pos, 0},
			{pos + glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), transform.z), 0},
			debug_color);
	}
}

void ZygoteShell::updateCachedDynamicPropsFromBody() {
	PhysicsProperties &props = cachedProps_;
	props.angle = physBody_.b2Body_->GetAngle();
	props.angularVelocity = physBody_.b2Body_->GetAngularVelocity();
	props.position = b2g(physBody_.b2Body_->GetPosition());
	props.velocity = b2g(physBody_.b2Body_->GetLinearVelocity());
}

void ZygoteShell::die() {
	dead_ = true;
}
