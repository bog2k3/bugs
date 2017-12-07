/*
 * Gripper.h
 *
 *  Created on: Nov 27, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_GRIPPER_H_
#define OBJECTS_BODY_PARTS_GRIPPER_H_

#include "BodyPart.h"
#include "../entities/Bug/IMotor.h"

#include <memory>
#include <atomic>

class b2WeldJoint;

struct GripperInitializationData : public BodyPartInitializationData {
	virtual ~GripperInitializationData() noexcept = default;
	GripperInitializationData() = default;

	CummulativeValue inputVMSCoord; // input nerve VMS coordinate
};


class Gripper : public BodyPart, public IMotor {
public:
	Gripper();
	~Gripper() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;

	void update(float dt);

	// IMotor::
	unsigned getInputCount() const override { return 1; }
	InputSocket* getInputSocket(unsigned index) const override { return index==0 ? inputSocket_ : 0; }
	float getInputVMSCoord(unsigned index) const override;
#ifdef DEBUG
	//std::string getMotorDebugName() const override { return getDebugName(); }
#endif

protected:
	InputSocket* inputSocket_;
	std::atomic<bool> active_;
	b2WeldJoint* groundJoint_;

	void setActive(bool active);
	void commit() override;
	void die() override;
	//void onAddedToParent() override;
};

#endif /* OBJECTS_BODY_PARTS_GRIPPER_H_ */
