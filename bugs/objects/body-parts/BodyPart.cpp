/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include <assert.h>

BodyPart::BodyPart(BodyPart* parent, PART_TYPE type, PhysicsProperties props)
	: WorldObject(props)
	, type_(type)
	, parent_(parent)
	, children_{nullptr}
	, nChildren_(0)
{
	if (parent)
		parent->add(this);
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
