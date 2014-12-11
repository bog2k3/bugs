/*
 * Joint.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#include "Joint.h"
#include <Box2D/Box2D.h>
#include "../WorldObject.h"
#include "../../math/box2glm.h"
#include "../../math/math2D.h"

Joint::Joint(BodyPart* parent, PhysicsProperties props)
	: BodyPart(parent, BODY_PART_JOINT, props)
	, offset1_(0)
	, offset2_(0)
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
	def.bodyA = parent_->getBody();
	def.localAnchorA = g2b(offset1_);
	def.bodyB = children_[0]->getBody();
	def.localAnchorB = g2b(offset2_);
	def.enableLimit = true;
	def.lowerAngle = phiMin_;
	def.upperAngle = phiMax_;
	def.userData = (void*)this;

	physJoint_ = (b2RevoluteJoint*)parent_->getBody()->GetWorld()->CreateJoint(&def);
}
