/*
 * Mouth.h
 *
 *  Created on: Jan 18, 2015
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_MOUTH_H_
#define OBJECTS_BODY_PARTS_MOUTH_H_

#include "BodyPart.h"

class b2WeldJoint;

class Mouth: public BodyPart {
public:
	Mouth(BodyPart* parent);
	virtual ~Mouth() override;

	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;
	void draw(RenderContext const& ctx) override;
	void update(float dt);

	void setProcessingSpeed(float massPerTime);

protected:
	float width_;
	float bufferSize_;
	float usedBuffer_;
	float processingSpeed_;		// [kg/s]
	b2WeldJoint* pJoint;

	void commit() override;
	void onCollision(PhysicsBody* pOther, float impulseMagnitude);
};

#endif /* OBJECTS_BODY_PARTS_MOUTH_H_ */
