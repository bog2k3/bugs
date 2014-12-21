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
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

static const glm::vec3 debug_color(1.f, 0.3f, 0.1f);

Joint::Joint(BodyPart* parent, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_JOINT, props)
	, size_(1.e-4f)
	, phiMin_(-PI/8)
	, phiMax_(PI * 0.9f)
	, physJoint_(nullptr)
{
}

Joint::~Joint() {
	// delete joint
}

void Joint::commit() {
	assert(nChildren_ == 1);

	b2RevoluteJointDef def;
	def.Initialize(parent_->getBody(), children_[0]->getBody(), g2b(vec3xy(getWorldTransformation())));
	// physProps_.position must be in world space at this step:
	def.enableLimit = true;
	def.lowerAngle = phiMin_;
	def.upperAngle = phiMax_;
	def.userData = (void*)this;

	physJoint_ = (b2RevoluteJoint*)World::getInstance()->getPhysics()->CreateJoint(&def);
}

glm::vec3 Joint::getWorldTransformation() const {
	glm::vec3 parentTransform(parent_ ? parent_->getWorldTransformation() : glm::vec3(0));
	if (!committed_)
		return parentTransform + glm::vec3(glm::rotate(initialData_->position, parentTransform.z), initialData_->angle);
	else {
		return glm::vec3(b2g(physJoint_->GetAnchorA()+physJoint_->GetAnchorB())*0.5f,
				physJoint_->GetJointAngle()); // getjoint angle is not quite the right thing.
	}
}

void Joint::draw(ObjectRenderContext* ctx) {
	if (committed_) {
		// nothing, physics draws
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx->shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, debug_color);
		ctx->shape->drawLine(pos, pos+glm::rotate(glm::vec2(sqrtf(size_/PI), 0), transform.z), 0, debug_color);
	}
}

glm::vec2 Joint::getRelativeAttachmentPoint(float relativeAngle)
{
	assert(!committed_);
	return glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle);
}
