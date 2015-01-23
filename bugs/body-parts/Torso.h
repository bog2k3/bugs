/*
 * Torso.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_TORSO_H_
#define OBJECTS_BODY_PARTS_TORSO_H_

#include "BodyPart.h"

class Mouth;

#define DEBUG_DRAW_TORSO

class Torso : public BodyPart {
public:
	Torso(BodyPart* parent);
	~Torso() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) const override;
	void setInitialFatMass(float fat);
	void update(float dt);
	inline float getFatMass() { return fatMass_; }
	inline float getBufferedEnergy() { return energyBuffer_; }

	void consumeEnergy(float amount) override;
	void addProcessedFood(float mass) override;
	float getMass_tree() override { return cachedMassTree_; }

	void setMouth(Mouth* m) { mouth_ = m; }
	void replenishEnergyFromMass(float mass);

	Event<void(float mass)> onFoodEaten;

protected:
	float size_;
	float fatMass_;
	float lastCommittedTotalSizeInv_;
	float frameUsedEnergy_;
	float energyBuffer_;
	float maxEnergyBuffer_;
	float cachedMassTree_;
	Mouth* mouth_;

	void commit() override;
	void die() override;
};

#endif /* OBJECTS_BODY_PARTS_TORSO_H_ */
