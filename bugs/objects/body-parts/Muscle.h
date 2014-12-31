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

class Muscle: public BodyPart {
public:
	// the position and rotation in props are relative to the parent:
	Muscle(BodyPart* parent, Joint* joint, int motorDirSign);
	virtual ~Muscle() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	float getSize() { return size_; }
	float getAspectRatio() { return aspectRatio_; }

protected:
	static const float contractionRatio;			// [1]
	static const float forcePerWidthRatio;			// [N/m]
	static const float maxLinearContractionSpeed;	// [m/s]

	Joint* joint_;
	float rotationSign_;
	CummulativeValue size_;
	CummulativeValue aspectRatio_;	// length/width

	float maxTorque_;
	float maxJointAngularSpeed_;
};

#endif /* OBJECTS_BODY_PARTS_MUSCLE_H_ */
