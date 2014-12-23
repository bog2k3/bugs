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
	virtual ~Bone() override;
	void commit() override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	void draw(RenderContext& ctx) override;

	float getDensity() { return density_; }
	float getSize() { return size_; }
	float getAspectRatio() { return aspectRatio_; }

protected:
	CummulativeValue density_;
	CummulativeValue size_;
	CummulativeValue aspectRatio_;
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
