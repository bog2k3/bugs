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
#include <memory>

struct BoneInitializationData : public BodyPartInitializationData {
	virtual ~BoneInitializationData() noexcept = default;
	BoneInitializationData();

	CummulativeValue aspectRatio;
};

class Bone: public BodyPart {
public:
	Bone();
	virtual ~Bone() override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	void draw(RenderContext const& ctx) override;
	float getAspectRatio();

protected:
	float length_;
	float width_;

	void commit() override;
	void cacheInitializationData() override;
	void die() override;
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
