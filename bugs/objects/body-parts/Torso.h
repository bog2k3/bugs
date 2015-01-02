/*
 * Torso.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_TORSO_H_
#define OBJECTS_BODY_PARTS_TORSO_H_

#include "BodyPart.h"

struct TorsoInitializationData : public BodyPartInitializationData {
	virtual ~TorsoInitializationData() noexcept = default;
	TorsoInitializationData() : density(1.f) {
	}

	CummulativeValue density;
};

class Torso : public BodyPart {
public:
	Torso(BodyPart* parent);
	~Torso() override;

	void commit() override;
	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;

protected:
	std::weak_ptr<TorsoInitializationData> torsoInitialData_;
	float size_;
	float density_;
};

#endif /* OBJECTS_BODY_PARTS_TORSO_H_ */
