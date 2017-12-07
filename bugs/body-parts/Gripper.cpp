/*
 * Gripper.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#include "../neuralnet/InputSocket.h"
#include "../ObjectTypesAndFlags.h"
#include "Gripper.h"
#include "BodyConst.h"

#include <boglfw/World.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/math/math3D.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/assert.h>
#include <boglfw/perf/marker.h>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

const glm::vec3 debug_color(1.f, 0.6f, 0.f);

#define DEBUG_DRAW_GRIPPER

Gripper::Gripper()
	: BodyPart(BodyPartType::GRIPPER, std::make_shared<GripperInitializationData>())
	, inputSocket_(new InputSocket(nullptr, 1.f))
	, active_(false)
	, groundJoint_(nullptr)
{
	getInitializationData()->density.reset(BodyConst::GripperDensity);

	auto initData = std::dynamic_pointer_cast<GripperInitializationData>(getInitializationData());
	registerAttribute(GENE_ATTRIB_MOTOR_INPUT_COORD, initData->inputVMSCoord);

	physBody_.userObjectType_ = ObjectTypes::BPART_GRIPPER;
	physBody_.userPointer_ = this;
}

float Gripper::getInputVMSCoord(unsigned index) const {
	if (index != 0)
		return 0;
	auto initData = std::dynamic_pointer_cast<GripperInitializationData>(getInitializationData());
	if (initData)
		return initData->inputVMSCoord.clamp(0, BodyConst::MaxVMSCoordinateValue);
	else
		return 0;
}

/*void Gripper::onAddedToParent() {
	assertDbg(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}*/

Gripper::~Gripper() {
	delete inputSocket_;
}

void Gripper::commit() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
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
	PERF_MARKER_FUNC;
	if (isDead())
		return;
	float intensity = inputSocket_->value;
	setActive(intensity > 0);
	// TODO: if force too big -> release gripper
}

void Gripper::setActive(bool active) {
	if (active_.load(std::memory_order_acquire) == active)
		return;
	active_.store(active, std::memory_order_release);
	if (active) {
		World::getInstance()->queueDeferredAction([this] {
			if (groundJoint_)
				return;
			b2WeldJointDef jd;
			jd.bodyA = World::getInstance()->getGroundBody();
			jd.localAnchorA = physBody_.b2Body_->GetWorldPoint(b2Vec2_zero);
			jd.bodyB = physBody_.b2Body_;
			groundJoint_ = (b2WeldJoint*)World::getInstance()->getPhysics()->CreateJoint(&jd);
		});
	} else {
		World::getInstance()->queueDeferredAction([this] {
			if (!groundJoint_)
				return;
			physBody_.b2Body_->GetWorld()->DestroyJoint(groundJoint_);
			groundJoint_ = nullptr;
		});
	}
}

void Gripper::draw(RenderContext const& ctx) {
	glm::vec3 transform = getWorldTransformation();
	glm::vec3 pos = {vec3xy(transform), 0};
	if (committed_) {
		// nothing, physics draws
#ifdef DEBUG_DRAW_GRIPPER
		if (isDead()) {
			float sizeLeft = getFoodValue() / density_;
			Shape3D::get()->drawCircleXOY(pos, sqrtf(sizeLeft*PI_INV)*0.6f, 12, glm::vec3(0.5f,0,1));
		} else if (active_) {
			Shape3D::get()->drawCircleXOY(pos, sqrtf(size_*PI_INV)*0.6f, 12, debug_color);
		}
#endif
	} else {
		Shape3D::get()->drawCircleXOY(pos, sqrtf(size_*PI_INV), 12, debug_color);
		Shape3D::get()->drawLine(pos,
				pos + glm::vec3(glm::rotate(glm::vec2(sqrtf(size_*PI_INV), 0), transform.z), 0),
				debug_color);
	}
}

glm::vec2 Gripper::getAttachmentPoint(float relativeAngle)
{
	if (!geneValuesCached_) {
#ifdef DEBUG
		World::assertOnMainThread();
#endif
		cacheInitializationData();
	}
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}

void Gripper::die() {
	setActive(false);
	if (context_)
		context_->updateList.remove(this);
}
