/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "../../math/box2glm.h"
#include "../../renderOpenGL/Shape2D.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <assert.h>

BodyPart::BodyPart(BodyPart* parent, PART_TYPE type, PhysicsProperties props)
	: WorldObject(props)
	, type_(type)
	, parent_(parent)
	, children_{nullptr}
	, nChildren_(0)
	, committed_(false)
{
	if (parent) {
		parent->add(this);
	}
}

BodyPart::~BodyPart() {
}

void BodyPart::add(BodyPart* part) {
	assert(nChildren_ < MAX_CHILDREN);
	children_[nChildren_++] = part;
}

void BodyPart::changeParent(BodyPart* newParent) {
	if (parent_)
		parent_->remove(this);
	parent_ = newParent;
	if (parent_)
		parent_->add(this);
}

void BodyPart::remove(BodyPart* part) {
	for (int i=0; i<nChildren_; i++)
		if (children_[i] == part) {
			children_[i] = children_[--nChildren_];
			break;
		}
}

void BodyPart::transform_position_and_angle() {
	if (parent_) {
		initialData_->position = b2g(parent_->body_->GetPosition()) +
				glm::rotate(initialData_->position, parent_->body_->GetAngle());
		initialData_->angle += parent_->body_->GetAngle();
	}
}

void BodyPart::commit_tree() {
	// perform commit on local node:
	assert(!committed_);
	// first transform position and angle into world space:
	transform_position_and_angle();
	WorldObject::commit();
	commit();
	WorldObject::purgeInitializationData();
	committed_ = true;
	// perform recursive commit on all children:
	for (auto c : children_)
		c->commit_tree();
}

glm::vec3 BodyPart::getWorldTransformation() {
	assert(!committed_);
	glm::vec3 parentTransform(parent_ ? parent_->getWorldTransformation() : glm::vec3(0));
	return parentTransform + glm::vec3(initialData_->position, initialData_->angle);
}

void BodyPart::draw(ObjectRenderContext* ctx) {
	if (committed_)
		return;
	glm::vec3 trans = getWorldTransformation();
	glm::vec2 pos(trans.x, trans.y);
	ctx->shape->drawLine(pos + glm::vec2(-0.01f, 0), pos + glm::vec2(0.01f, 0), 0, glm::vec3(0.2f, 0.2f, 1.f));
	ctx->shape->drawLine(pos + glm::vec2(0, -0.01f), pos + glm::vec2(0, 0.01f), 0, glm::vec3(1.f, 0.2f, 0.2f));
}
