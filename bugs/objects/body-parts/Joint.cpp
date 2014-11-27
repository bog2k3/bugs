/*
 * Joint.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#include "objects/body-parts/Joint.h"
#include "objects/body-parts/Bone.h"
#include <Box2D/Box2D.h>
#include "../../math/box2glm.h"

Joint::Joint(Bone* b1, glm::vec2 offset1, Bone* b2, glm::vec2 offset2, float size, float phiMin, float phiMax) {
	b2RevoluteJointDef def;
	def.bodyA = b1->getBody();
	def.localAnchorA = g2b(offset1);
	def.bodyB = b2->getBody();
	def.localAnchorB = g2b(offset2);
	def.enableLimit = true;
	def.lowerAngle = phiMin;
	def.upperAngle = phiMax;
	def.userData = (void*)this;

	b1->getBody()->GetWorld()->CreateJoint(&def);
}

Joint::~Joint() {
	// TODO Auto-generated destructor stub
}

