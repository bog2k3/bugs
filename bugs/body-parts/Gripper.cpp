/*
 * Gripper.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#include "Gripper.h"
#include "BodyConst.h"
#include "../World.h"
#include "../renderOpenGL/Shape2D.h"
#include "../math/math2D.h"
#include "../neuralnet/InputSocket.h"
#include "../utils/UpdateList.h"
#include "../utils/assert.h"
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "../utils/log.h"

const glm::vec3 debug_color(1.f, 0.6f, 0.f);

#define DEBUG_DRAW_GRIPPER

Gripper::Gripper()
	: BodyPart(BODY_PART_GRIPPER, std::make_shared<BodyPartInitializationData>())
	, inputSocket_(std::make_shared<InputSocket>(nullptr, 1.f))
	, active_(false)
	, groundJoint_(nullptr)
{
	getInitializationData()->density.reset(BodyConst::GripperDensity);

	physBody_.userObjectType_ = ObjectTypes::BPART_GRIPPER;
	physBody_.userPointer_ = this;
	physBody_.categoryFlags_ = EventCategoryFlags::BODYPART;
}

void Gripper::onAddedToParent() {
	assert(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}

Gripper::~Gripper() {
}

void Gripper::commit() {
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

void Gripper::update(float dt) {
	if (isDead())
		return;
	float intensity = inputSocket_->value;
	setActive(intensity > 0);
}

void Gripper::setActive(bool active) {
	if (active_ == active)
		return;
	active_ = active;
	if (active) {
		b2WeldJointDef jd;
		jd.bodyA = World::getInstance()->getGroundBody();
		jd.localAnchorA = physBody_.b2Body_->GetWorldPoint(b2Vec2_zero);
		jd.bodyB = physBody_.b2Body_;
		groundJoint_ = (b2WeldJoint*)World::getInstance()->getPhysics()->CreateJoint(&jd);
	} else {
		physBody_.b2Body_->GetWorld()->DestroyJoint(groundJoint_);
		groundJoint_ = nullptr;
	}
}

void Gripper::draw(RenderContext const& ctx) {
	if (committed_) {
		// nothing, physics draws
#ifdef DEBUG_DRAW_GRIPPER
		if (active_) {
			glm::vec3 transform = getWorldTransformation();
			glm::vec2 pos = vec3xy(transform);
			ctx.shape->drawCircle(pos, sqrtf(size_*PI_INV)*0.6f, 0, 12, debug_color);
		}
#endif
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_*PI_INV), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(size_*PI_INV), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 Gripper::getChildAttachmentPoint(float relativeAngle)
{
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle));
	assert(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

void Gripper::die() {
	setActive(false);
	if (getUpdateList())
		getUpdateList()->remove(this);
}

