/*
 * Muscle.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_MUSCLE_H_
#define OBJECTS_BODY_PARTS_MUSCLE_H_

#include "BodyPart.h"
#include "../../entities/bug_stuff/IMotor.h"

#define DEBUG_DRAW_MUSCLE

class Joint;

struct MuscleInitializationData : public BodyPartInitializationData {
	virtual ~MuscleInitializationData() noexcept = default;
	MuscleInitializationData();

	CummulativeValue aspectRatio;	// length/width
};

class Muscle: public BodyPart, public IMotor {
public:
	// the position and rotation in props are relative to the parent:
	Muscle(BodyPart* parent, Joint* joint, int motorDirSign);
	virtual ~Muscle() override;

	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;
	void update(float dt);

	virtual std::shared_ptr<InputSocket> getInputSocket() override { return inputSocket_; }

protected:
	static constexpr int nAngleSteps = 10;

	std::weak_ptr<MuscleInitializationData> muscleInitialData_;
	std::shared_ptr<InputSocket> inputSocket_;
	Joint* joint_;
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
	void commit() override;
};

#endif /* OBJECTS_BODY_PARTS_MUSCLE_H_ */
