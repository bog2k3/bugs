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
#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "../utils/log.h"

const glm::vec3 debug_color(1.f, 0.6f, 0.f);

Gripper::Gripper(BodyPart* parent)
	: BodyPart(parent, BODY_PART_GRIPPER, std::make_shared<BodyPartInitializationData>())
	, inputSocket_(std::make_shared<InputSocket>(nullptr, 1.f))
	, active_(false)
	, groundJoint_(nullptr)
	, size_(0)
{
	getInitializationData()->density.reset(BodyConst::GripperDensity);

	getUpdateList()->add(this);
}

Gripper::~Gripper() {
	setActive(false);
}

void Gripper::commit() {
	if (committed_) {
		physBody_.b2Body_->DestroyFixture(&physBody_.b2Body_->GetFixtureList()[0]);
	};

	std::shared_ptr<BodyPartInitializationData> initData = getInitializationData();
	size_ = initData->size;

	b2CircleShape shape;
	shape.m_radius = sqrtf(size_ * PI_INV);
	b2FixtureDef fdef;
	fdef.density = initData->density;
	fdef.friction = 0.3f;
	fdef.restitution = 0.2f;
	fdef.shape = &shape;
	physBody_.b2Body_->CreateFixture(&fdef);
}

void Gripper::update(float dt) {
	float intensity = inputSocket_->value;
	setActive(intensity > 0.5f);
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
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(getInitializationData()->size/PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(getInitializationData()->size/PI), 0), transform.z),
				0, debug_color);
	}
}

glm::vec2 Gripper::getChildAttachmentPoint(float relativeAngle) const
{
	float size = size_;
	if (!committed_) {
		std::shared_ptr<BodyPartInitializationData> initData = getInitializationData();
		size = initData->size;
	}
	return glm::rotate(glm::vec2(sqrtf(size * PI_INV), 0), relativeAngle);
}

void Gripper::die() {
	setActive(false);
}
