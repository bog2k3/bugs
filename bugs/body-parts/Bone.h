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
	void draw(RenderContext const& ctx) override;

	float aspectRatio() const { return length_ / width_; }

protected:
	float length_;
	float width_;

	void updateFixtures() override;

	static float getDensity(BodyCell const& cell);	// return the density of the future body part created from this cell
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
