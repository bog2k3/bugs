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

struct MouthInitializationData : public BodyPartInitializationData {
	virtual ~MouthInitializationData() noexcept = default;
	MouthInitializationData();

	CummulativeValue aspectRatio;
};

class Mouth: public BodyPart {
public:
	Mouth();
	virtual ~Mouth() override;

	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	void draw(RenderContext const& ctx) override;
	void update(float dt);

protected:
	float length_;
	float width_;
	float bufferSize_;
	float usedBuffer_;
	b2WeldJoint* pJoint;

	void cacheInitializationData() override;
	void commit() override;
	void onCollision(PhysicsBody* pOther, float impulseMagnitude);
	void onAddedToParent() override;
	void die() override;
};

#endif /* OBJECTS_BODY_PARTS_MOUTH_H_ */
