/*
 * Gripper.h
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_GRIPPER_H_
#define OBJECTS_BODY_PARTS_GRIPPER_H_

#include "BodyPart.h"
#include "../../entities/IMotor.h"

class b2WeldJoint;

class Gripper : public BodyPart, public IMotor {
public:
	// the position and rotation in props are relative to the parent
	Gripper(BodyPart* parent);
	~Gripper() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;

	/**
	 * command the motor with the given intensity;
	 * intensity depends on the type and properties of the motor
	 */
	virtual void action(float intensity);

protected:
	void setActive(bool active);

	bool active_;
	b2WeldJoint* groundJoint_;
	float size_;
};

#endif /* OBJECTS_BODY_PARTS_GRIPPER_H_ */
