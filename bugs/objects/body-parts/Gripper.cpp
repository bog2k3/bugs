/*
 * Gripper.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#include "Gripper.h"
#include "../../World.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../math/math2D.h"
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

const glm::vec3 debug_color(1.f, 0.6f, 0.f);

Gripper::Gripper(BodyPart* parent, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_GRIPPER, props)
	, size_(0.5e-4f)
	, density_(1.f)
	, active_(false)
	, groundJoint_(nullptr)
{
}

Gripper::~Gripper() {
	setActive(false);
}

void Gripper::commit() {
	assert(!committed_);
	b2CircleShape shape;
	shape.m_radius = sqrtf(size_ * PI_INV);
	b2FixtureDef fdef;
	fdef.density = density_;
	fdef.friction = 0.3f;
	fdef.restitution = 0.2f;
	fdef.shape = &shape;
	body_->CreateFixture(&fdef);
}

void Gripper::setSize(float value) {
	assert(!committed_);
	size_ = value;
}
void Gripper::setDensity(float value) {
	assert(!committed_);
	density_ = value;
}

void Gripper::setActive(bool active) {
	if (active_ == active)
		return;
	active_ = active;
	if (active) {
		b2WeldJointDef jd;
		jd.bodyA = World::getInstance()->getGroundBody();
		jd.localAnchorA = body_->GetWorldPoint(b2Vec2_zero);
		jd.bodyB = body_;
		groundJoint_ = (b2WeldJoint*)World::getInstance()->getPhysics()->CreateJoint(&jd);
	} else {
		body_->GetWorld()->DestroyJoint(groundJoint_);
		groundJoint_ = nullptr;
	}
}

void Gripper::draw(RenderContext& ctx) {
	if (committed_) {
		// nothing, physics draws
	} else {
		initialData_->position = getFinalPrecommitPosition();
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos, pos + glm::rotate(glm::vec2(sqrtf(size_/PI), 0), transform.z), 0, debug_color);
	}
}

glm::vec2 Gripper::getChildAttachmentPoint(float relativeAngle)
{
	return glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle);
}
