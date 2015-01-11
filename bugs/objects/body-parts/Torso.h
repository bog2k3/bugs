/*
 * Torso.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_TORSO_H_
#define OBJECTS_BODY_PARTS_TORSO_H_

#include "BodyPart.h"

#define DEBUG_DRAW_TORSO

class Torso : public BodyPart {
public:
	Torso(BodyPart* parent);
	~Torso() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;
	void setFatMass(float fat);

protected:
	float size_;
	float fatMass_;
};

#endif /* OBJECTS_BODY_PARTS_TORSO_H_ */
