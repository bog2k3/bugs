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
	Muscle(BodyPart* parent, PhysicsProperties props, Joint* joint); // the position and rotation in props are relative to the parent
	virtual ~Muscle() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	float getSize() { return size_; }
	float getAspectRatio() { return aspectRatio_; }

protected:
	static const float contractionRatio;
	static const float forcePerWidthRatio;

	Joint* joint_;
	CummulativeValue size_;
	CummulativeValue aspectRatio_;	// length/width

	float maxTorque_;
};

#endif /* OBJECTS_BODY_PARTS_MUSCLE_H_ */
