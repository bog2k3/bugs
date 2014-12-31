/*
 * Joint.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#include "Joint.h"
#include "../../World.h"
#include "../../math/box2glm.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../log.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(1.f, 0.3f, 0.1f);

Joint::Joint(BodyPart* parent)
	: BodyPart(parent, BODY_PART_JOINT)
	, size_(0.2e-4f)
	, phiMin_(-PI/8)
	, phiMax_(PI * 0.9f)
	, physJoint_(nullptr)
{
}

Joint::~Joint() {
	// delete joint
}

/**
 * if the angles are screwed, limit them to 0
 */
void Joint::fixAngles() {
	if (phiMin_ > 0)
		phiMin_ = 0;
	if (phiMax_ < 0)
		phiMax_ = 0;
}

void Joint::commit() {
	assert(!committed_);
	assert(nChildren_ == 1);

	fixAngles();

	b2RevoluteJointDef def;
	// physProps_.position is be in world space at this step:
	def.Initialize(parent_->getBody(), children_[0]->getBody(), g2b(initialData_->position));
	def.enableLimit = true;
	def.lowerAngle = phiMin_;
	def.upperAngle = phiMax_;
	def.userData = (void*)this;
	// def.collideConnected = true;

	physJoint_ = (b2RevoluteJoint*)World::getInstance()->getPhysics()->CreateJoint(&def);
}

glm::vec3 Joint::getWorldTransformation() const {
	if (!committed_)
		return BodyPart::getWorldTransformation();
	else {
		return glm::vec3(b2g(physJoint_->GetAnchorA()+physJoint_->GetAnchorB())*0.5f,
			physJoint_->GetBodyA()->GetAngle() + physJoint_->GetJointAngle());
	}
}

void Joint::draw(RenderContext& ctx) {
	if (committed_) {
		// nothing, physics draws
	} else {
		initialData_->position = getFinalPrecommitPosition();
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos, pos+glm::rotate(glm::vec2(sqrtf(size_/PI), 0), transform.z), 0, debug_color);
	}
}

glm::vec2 Joint::getChildAttachmentPoint(float relativeAngle)
{
	return glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle);
}

float Joint::getTotalRange() {
	if (committed_) {
		// the angles are already fixed
		return phiMax_ - phiMin_;
	} else {
		// save original values:
		float fm = phiMin_, fM = phiMax_;
		// fix the angles:
		fixAngles();
		// compute range:
		float ret = phiMax_ - phiMin_;
		// restore original values:
		phiMax_ = fM, phiMin_ = fm;

		return ret;
	}
}
