/*
 * Joint.h
 *
 *  Created on: Nov 27, 2014
 *      Author: bogdan
 */

#ifndef OBJECTS_BODY_PARTS_JOINT_H_
#define OBJECTS_BODY_PARTS_JOINT_H_

#include <glm/fwd.hpp>
#include "BodyPart.h"

class b2RevoluteJoint;

class Joint {
public:
	Joint(BodyPart* parent, glm::vec2 offset1, BodyPart* other, glm::vec2 offset2, float size, float phiMin, float phiMax);
	virtual ~Joint();

protected:
	b2RevoluteJoint* physJoint;
};

#endif /* OBJECTS_BODY_PARTS_JOINT_H_ */
