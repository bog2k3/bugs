/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include <glm/gtx/rotate_vector.hpp>
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
		physProps_->position = parent_->physProps_->position + glm::rotate(physProps_->position, parent_->physProps_->angle);
		physProps_->angle += parent_->physProps_->angle;
	}
}

void BodyPart::commit_tree() {
	// first transform position and angle into world space:
	transform_position_and_angle();
	// perform commit on local node:
	assert(!committed_);
	WorldObject::commit();
	commit();
	committed_ = true;
	// perform recursive commit on all children:
	for (auto c : children_)
		c->commit_tree();
}
