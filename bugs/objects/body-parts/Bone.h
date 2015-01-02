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
	BoneInitializationData()
		: density(1) , aspectRatio(0.7f) {
	}

	CummulativeValue density;
	CummulativeValue aspectRatio;
};

class Bone: public BodyPart {
public:
	// the position and rotation in props are relative to the parent
	Bone(BodyPart* parent);
	virtual ~Bone() override;
	void commit() override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;

	void draw(RenderContext& ctx) override;

protected:
	std::weak_ptr<BoneInitializationData> boneInitialData_;
	glm::vec2 size_;
	float density_;
};

#endif /* OBJECTS_BODY_PARTS_BONE_H_ */
