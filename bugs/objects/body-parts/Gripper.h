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
	Gripper(BodyPart* parent, PhysicsProperties props);
	virtual ~Gripper() override;

	virtual void commit() override;

	void setActive(bool active);
	bool isActive() { return active_; }

	float getRadius() { return radius_; }
	float getDensity() { return density_; }

	void setRadius(float value);
	void setDensity(float value);

protected:
	float radius_;
	float density_;
	bool active_;
	bool committed_;
	b2WeldJoint* groundJoint_;
};

#endif /* OBJECTS_BODY_PARTS_GRIPPER_H_ */
