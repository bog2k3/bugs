/*
 * Joint.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#include "Joint.h"
#include "../WorldObject.h"
#include "../../math/box2glm.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include <Box2D/Box2D.h>
#include <glm/gtx/rotate_vector.hpp>

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
	def.Initialize(parent_->getBody(), children_[0]->getBody(), g2b(initialData_->position));
	// physProps_.position must be in world space at this step:
	def.enableLimit = true;
	def.lowerAngle = phiMin_;
	def.upperAngle = phiMax_;
	def.userData = (void*)this;

	physJoint_ = (b2RevoluteJoint*)parent_->getBody()->GetWorld()->CreateJoint(&def);
}

void Joint::draw(ObjectRenderContext* ctx) {
	if (committed_) {
		// nothing, physics draws
	} else {
		glm::vec2 pos = vec3xy(getWorldTransformation());
		ctx->shape->drawCircle(pos, sqrtf(size_/PI), 0, 12, glm::vec3(0.4f, 1.f, 0.f));
	}
}

glm::vec2 Joint::getRelativeAttachmentPoint(float relativeAngle)
{
	assert(!committed_);
	return glm::rotate(glm::vec2(0, sqrtf(size_ * PI_INV)), relativeAngle);
}
