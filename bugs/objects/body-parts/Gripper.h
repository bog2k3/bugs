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

struct GripperInitializationData : public BodyPartInitializationData {
	virtual ~GripperInitializationData() noexcept = default;
	GripperInitializationData()
		: density(1) {
	}

	CummulativeValue density;
};

class Gripper : public BodyPart {
public:
	// the position and rotation in props are relative to the parent
	Gripper(BodyPart* parent);
	~Gripper() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	void setActive(bool active);
	bool isActive() { return active_; }

protected:
	std::weak_ptr<GripperInitializationData> gripperInitialData_;
	bool active_;
	b2WeldJoint* groundJoint_;
};

#endif /* OBJECTS_BODY_PARTS_GRIPPER_H_ */
