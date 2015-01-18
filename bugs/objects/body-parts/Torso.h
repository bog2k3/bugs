/*
 * Torso.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_TORSO_H_
#define OBJECTS_BODY_PARTS_TORSO_H_

#include "BodyPart.h"
#include "../../updatable.h"

#define DEBUG_DRAW_TORSO

class Torso : public BodyPart {
public:
	Torso(BodyPart* parent);
	~Torso() override;

	void draw(RenderContext& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;
	void setInitialFatMass(float fat);
	void update(float dt);
	inline float getFatMass() { return fatMass_; }

	void consumeEnergy(float amount) override;

protected:
	float size_;
	float fatMass_;
	float lastCommittedTotalSizeInv_;
	float frameUsedEnergy_;

	void commit() override;
	void die() override;
};

template<> void update(Torso* &t, float dt);

#endif /* OBJECTS_BODY_PARTS_TORSO_H_ */
