/*
 * Muscle.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_MUSCLE_H_
#define OBJECTS_BODY_PARTS_MUSCLE_H_

#include "BodyPart.h"

#define DEBUG_DRAW_MUSCLE

class Joint;

struct MuscleInitializationData : public BodyPartInitializationData {
	virtual ~MuscleInitializationData() noexcept = default;
	MuscleInitializationData() : aspectRatio(2.0f) {
	}

	CummulativeValue aspectRatio;	// length/width
};

class Muscle: public BodyPart {
public:
	// the position and rotation in props are relative to the parent:
	Muscle(BodyPart* parent, Joint* joint, int motorDirSign);
	virtual ~Muscle() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;

	/**
	 * command the muscle to contract. signal_strength will be clamped to [0.0, 1.0]
	 */
	void command(float signal_strength);

protected:
	static constexpr float contractionRatio = 0.5f;			// [1]
	static constexpr float forcePerWidthRatio = 100;		// [N/m] the theoretical force of a muscle 1 meter wide.
	static constexpr float maxLinearContractionSpeed = 0.8f;// [m/s] max meters/second linear contraction speed
	static constexpr int nAngleSteps = 10;					// [1]

	/**
	 * returns a float.
	 * [ret] - is current slice
	 * {ret} is relative position in current slice [0.0, 1.0] - use this to interpolate
	 */
	float getCurrentPhiSlice();

	std::weak_ptr<MuscleInitializationData> muscleInitialData_;
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
};

#endif /* OBJECTS_BODY_PARTS_MUSCLE_H_ */
