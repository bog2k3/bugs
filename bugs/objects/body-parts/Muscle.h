/*
 * Muscle.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_MUSCLE_H_
#define OBJECTS_BODY_PARTS_MUSCLE_H_

#include "BodyPart.h"

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
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

protected:
	static const float contractionRatio;			// [1]
	static const float forcePerWidthRatio;			// [N/m]
	static const float maxLinearContractionSpeed;	// [m/s]

	std::weak_ptr<MuscleInitializationData> muscleInitialData_;
	Joint* joint_;
	float rotationSign_;
	float maxTorque_;
	float maxJointAngularSpeed_;
};

#endif /* OBJECTS_BODY_PARTS_MUSCLE_H_ */
