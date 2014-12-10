/*
 * Gripper.h
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_GRIPPER_H_
#define OBJECTS_BODY_PARTS_GRIPPER_H_

#include "BodyPart.h"

class b2WeldJoint;

class Gripper : public BodyPart {
public:
	Gripper(BodyPart* parent, float radius, float density, PhysicsProperties props);
	virtual ~Gripper();

	void setActive(bool active);

protected:
	float radius;
	bool active;
	b2WeldJoint* groundJoint;
};

#endif /* OBJECTS_BODY_PARTS_GRIPPER_H_ */
