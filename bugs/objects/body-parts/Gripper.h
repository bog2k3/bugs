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
	// the position and rotation in props are relative to the parent
	Gripper(BodyPart* parent, PhysicsProperties props);
	~Gripper() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	void setActive(bool active);
	bool isActive() { return active_; }

	float getDensity() { return density_; }
	float getSize() { return size_; }

	void setDensity(float value);
	void setSize(float value);

protected:
	float size_;
	float density_;
	bool active_;
	b2WeldJoint* groundJoint_;
};

#endif /* OBJECTS_BODY_PARTS_GRIPPER_H_ */
