/*
 * Joint.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#include "Joint.h"
#include "../WorldObject.h"
#include <Box2D/Box2D.h>
#include "../../math/box2glm.h"

Joint::Joint(WorldObject* b1, glm::vec2 offset1, WorldObject* b2, glm::vec2 offset2, float size, float phiMin, float phiMax) {
	b2RevoluteJointDef def;
	def.bodyA = b1->getBody();
	def.localAnchorA = g2b(offset1);
	def.bodyB = b2->getBody();
	def.localAnchorB = g2b(offset2);
	def.enableLimit = true;
	def.lowerAngle = phiMin;
	def.upperAngle = phiMax;
	def.userData = (void*)this;

	physJoint = (b2RevoluteJoint*)b1->getBody()->GetWorld()->CreateJoint(&def);
}

Joint::~Joint() {
	// TODO Auto-generated destructor stub
}

