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
	// the position and rotation in props are relative to the parent
	Bone(BodyPart* parent, PhysicsProperties props);
	~Bone() override;
	void commit() override;
	glm::vec2 getRelativeAttachmentPoint(float relativeAngle) override;

	void draw(ObjectRenderContext* ctx) override;

	float getDensity() { return density_; }
	glm::vec2 getSize() { return size_; }

	void setDensity(float value);
	void setSize(glm::vec2 value);

protected:
	float density_;
	glm::vec2 size_;
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
