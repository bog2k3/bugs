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
	Mouth(BodyPart* parent);
	virtual ~Mouth() override;

	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;
	void draw(RenderContext& ctx) override;

protected:
	float linearSize_;

	void commit() override;
};

#endif /* OBJECTS_BODY_PARTS_MOUTH_H_ */
