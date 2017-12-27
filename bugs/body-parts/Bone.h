/*
 * Bone.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BONE_H_
#define OBJECTS_BODY_PARTS_BONE_H_

#include "BodyPart.h"

#include <glm/vec2.hpp>

class Bone: public BodyPart {
public:
	Bone();
	virtual ~Bone() override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;

	void draw(RenderContext const& ctx) override;
	float getAspectRatio();

protected:
	float length_;
	float width_;

	void updateFixtures() override;
	void die() override;
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
