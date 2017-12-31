/*
 * Mouth.h
 *
 *  Created on: Jan 18, 2015
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_MOUTH_H_
#define OBJECTS_BODY_PARTS_MOUTH_H_

#include "BodyPart.h"

class Mouth: public BodyPart {
public:
	Mouth(BodyPartContext const& context, BodyCell& cell);
	virtual ~Mouth() override;

	glm::vec2 getAttachmentPoint(float relativeAngle) override;
	void draw(RenderContext const& ctx) override;
	void update(float dt);

	static float getDensity(BodyCell const& cell);

protected:
	float length_;
	float width_;
	float bufferSize_;
	float usedBuffer_;
//	b2WeldJoint* pJoint;
	int onCollisionEventHandle_;

	void updateFixtures() override;
	void onCollision(PhysicsBody* pOther, float impulseMagnitude);
	void die() override;
};

#endif /* OBJECTS_BODY_PARTS_MOUTH_H_ */
