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

class Joint;
class InputSocket;

struct MuscleInitializationData : public BodyPartInitializationData {
	virtual ~MuscleInitializationData() noexcept = default;
	MuscleInitializationData();

	CummulativeValue aspectRatio;	// length/width
	CummulativeValue inputVMSCoord = 0; // input nerve VMS coordinate
};

class Muscle: public BodyPart, public IMotor {
public:
	// the position and rotation in props are relative to the parent:
	Muscle(Joint* joint, int motorDirSign);
	virtual ~Muscle() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	void update(float dt);

	// IMotor::
	std::shared_ptr<InputSocket> getInputSocket() const override { return inputSocket_; }
	float getInputVMSCoord() const override;

protected:
	static constexpr int nAngleSteps = 10;

	std::shared_ptr<InputSocket> inputSocket_;
	Joint* joint_;
	float aspectRatio_;
	float rotationSign_;
	float maxForce_;
	float maxJointAngularSpeed_;
	float phiToRSinAlphaHSinBeta_[nAngleSteps];	// r*sin(alpha)+h*sin(beta) = f(phi) table
	float phiAngleStep_;						// angle increment for each step of the nAngleSteps slices of phi range
	float cachedPhiMin_;

#ifdef DEBUG_DRAW_MUSCLE
	float phiToDx_[nAngleSteps];
#endif

	/**
	 * returns a float.
	 * [ret] - is current slice
	 * {ret} is relative position in current slice [0.0, 1.0] - use this to interpolate
	 */
	float getCurrentPhiSlice();
	void cacheInitializationData() override;
	void commit() override;
	void onAddedToParent() override;
	void die() override;

private:
	void onJointDied(BodyPart* j);
};

#endif /* OBJECTS_BODY_PARTS_MUSCLE_H_ */
