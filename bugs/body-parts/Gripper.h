/*
 * Gripper.h
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_GRIPPER_H_
#define OBJECTS_BODY_PARTS_GRIPPER_H_

#include "BodyPart.h"
#include <memory>

class b2WeldJoint;
class InputSocket;

class Gripper : public BodyPart {
public:
	Gripper();
	~Gripper() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	void update(float dt);

	std::shared_ptr<InputSocket> getInputSocket() { return inputSocket_; }

protected:
	std::shared_ptr<InputSocket> inputSocket_;
	bool active_;
	b2WeldJoint* groundJoint_;

	void setActive(bool active);
	void commit() override;
	void die() override;
	void onAddedToParent() override;
};

#endif /* OBJECTS_BODY_PARTS_GRIPPER_H_ */
