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
	Bone(BodyPartContext const& context, BodyCell& cell);
	virtual ~Bone() override;

	glm::vec2 getAttachmentPoint(float relativeAngle) override;
	void draw(Viewport* vp) override;

	float aspectRatio() const { return length_ / width_; }

	static float getDensity(BodyCell const& cell);	// return the density of the future body part created from this cell
	static float getRadius(BodyCell const& cell, float angle);

protected:
	float length_;
	float width_;

	void updateFixtures() override;

	static float extractAspectRatio(BodyCell const& cell);
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
