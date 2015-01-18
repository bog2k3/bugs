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
#include "../../updatable.h"

class b2WeldJoint;

class Gripper : public BodyPart, public IMotor {
public:
	// the position and rotation in props are relative to the parent
	Gripper(BodyPart* parent);
	~Gripper() override;

	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;

	void update(float dt);

	std::shared_ptr<InputSocket> getInputSocket() override { return inputSocket_; }

protected:
	std::shared_ptr<InputSocket> inputSocket_;
	bool active_;
	b2WeldJoint* groundJoint_;
	float size_;

	void setActive(bool active);
	void commit() override;
};

template<> void update(Gripper* &g, float dt);

#endif /* OBJECTS_BODY_PARTS_GRIPPER_H_ */
