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
	CummulativeValue inputVMSCoord; // input nerve VMS coordinate
};

class Muscle: public BodyPart, public IMotor {
public:
	// the position and rotation in props are relative to the parent:
	Muscle();
	virtual ~Muscle() override;

	void setJoint(Joint* joint, int motorDirSign);

	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	void update(float dt);

	// IMotor::
	unsigned getInputCount() const override { return 1; }
	InputSocket* getInputSocket(unsigned index) const override { return index==0 ? inputSocket_ : nullptr; }
	float getInputVMSCoord(unsigned index) const override;
#ifdef DEBUG
	std::string getMotorDebugName() const override { return getDebugName(); }
#endif

protected:
	static constexpr int nAngleSteps = 10;

	InputSocket* inputSocket_ = nullptr;
	Joint* joint_ = nullptr;
	float aspectRatio_;
	float rotationSign_ = 1.f;
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
