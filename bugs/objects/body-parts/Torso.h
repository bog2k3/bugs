/*
 * Torso.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_TORSO_H_
#define OBJECTS_BODY_PARTS_TORSO_H_

#include "BodyPart.h"

class Torso : public BodyPart {
public:
	Torso(BodyPart* parent, PhysicsProperties props);
	~Torso() override;

	void commit() override;
	void draw(RenderContext* ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	// returns the 'size' (surface area)
	float getSize() { return size_; }
	float getDensity() { return density_; }

protected:
	CummulativeValue size_;
	CummulativeValue density_;
};

#endif /* OBJECTS_BODY_PARTS_TORSO_H_ */
