/*
 * Muscle.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_MUSCLE_H_
#define OBJECTS_BODY_PARTS_MUSCLE_H_

#include "BodyPart.h"
#include "../entities/Bug/IMotor.h"

#define DEBUG_DRAW_MUSCLE

class JointPivot;
class InputSocket;

class Muscle: public BodyPart, public IMotor {
public:
	// the position and rotation in props are relative to the parent:
	Muscle(BodyPartContext const& context, BodyCell& cell, bool isRightSide);
	virtual ~Muscle() override;

	void setJoint(JointPivot* joint);

	void draw(RenderContext const& ctx) override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;
	void update(float dt);

	glm::vec3 getWorldTransformation() const override;

	// IMotor::
	unsigned getInputCount() const override { return 1; }
	InputSocket* getInputSocket(unsigned index) const override { return index==0 ? inputSocket_ : nullptr; }
	float getInputVMSCoord(unsigned index) const override { return inputVMSCoord_; }
#ifdef DEBUG
	std::string getMotorDebugName() const override { return getDebugName(); }
#endif

protected:
	static constexpr int nAngleSteps = 10;

	InputSocket* inputSocket_ = nullptr;
	JointPivot* joint_ = nullptr;
	float inputVMSCoord_;
	float aspectRatio_;
	float rotationSign_;
	float maxForce_;
	float maxJointAngularSpeed_;
	float H_phi_[nAngleSteps];	// H(phi) table
	float phiAngleStep_;		// angle increment for each step of the nAngleSteps slices of phi range
	float cachedPhiMin_;
	float insertionAngle_[2];

	glm::vec3 anchor_ {0};	// anchor (first insertion point) into left cell's coordinate frame

#ifdef DEBUG_DRAW_MUSCLE
	float phiToL_[nAngleSteps];
	bool isRightSide_;
#endif

	/**
	 * returns a float.
	 * [ret] - is current slice
	 * {ret} is relative position in current slice [0.0, 1.0] - use this to interpolate
	 */
	float getCurrentPhiSlice();
//	void cacheInitializationData() override;
	void updateFixtures() override;
	//void onAddedToParent() override;

private:
	void onJointDied(BodyPart* j);
};

#endif /* OBJECTS_BODY_PARTS_MUSCLE_H_ */
