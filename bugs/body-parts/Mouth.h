/*
 * Mouth.h
 *
 *  Created on: Jan 18, 2015
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_MOUTH_H_
#define OBJECTS_BODY_PARTS_MOUTH_H_

#include "BodyPart.h"

#include <glm/vec2.hpp>

class Mouth: public BodyPart {
public:
	Mouth(BodyPartContext const& context, BodyCell& cell);
	virtual ~Mouth() override;

	glm::vec2 getAttachmentPoint(float relativeAngle) override;
	void draw(RenderContext const& ctx) override;
	void update(float dt);

	static float getDensity(BodyCell const& cell);
	static float getRadius(BodyCell const& cell, float angle);

protected:
	float length_;
	float width_;
	float bufferSize_;
	float usedBuffer_;
//	b2WeldJoint* pJoint;
	int onCollisionEventHandle_;

	float lastDt_ = 0;

	void updateFixtures() override;
	void onCollision(PhysicsBody* pOther, float impulseMagnitude);
	void die() override;

	static float extractAspectRatio(BodyCell const& cell);
};

#endif /* OBJECTS_BODY_PARTS_MOUTH_H_ */
