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
	Torso();
	~Torso() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	void setInitialFatMass(float fat);
	void update(float dt);
	inline float getFatMass() { return fatMass_; }
	inline float getBufferedEnergy() { return energyBuffer_; }
	inline void setExtraMass(float mass) { extraMass_ = mass; }

	void consumeEnergy(float amount) override;
	float addFood(float mass) override;
	// body mass doesn't include fat mass as long as initializationData is available. b2Body mass does include fat too.
	float getMass_tree() override {
		if (cachedMassTree_ == 0)
			cachedMassTree_ = BodyPart::getMass_tree();
		return cachedMassTree_;
	}
	void applyScale_tree(float scale) override {
		cachedMassTree_ = 0;	// reset the cached value to force a recomputation
		BodyPart::applyScale_tree(scale);
	}
	void detach(bool die) override;

	void setMouth(Mouth* m) { mouth_ = m; }
	void replenishEnergyFromMass(float mass);

	Event<void(float mass)> onFoodProcessed;
	Event<void(std::vector<int> const&)> onMotorLinesDetached;

protected:
	float fatMass_;
	float lastCommittedTotalSizeInv_;
	float frameUsedEnergy_;
	float energyBuffer_;
	float maxEnergyBuffer_;
	float cachedMassTree_;
	float extraMass_;
	Mouth* mouth_;
	float foodProcessingSpeed_;
	float foodBufferSize_;
	float foodBuffer_;

	void commit() override;
	void die() override;
	void onAddedToParent() override;
	void detachMotorLines(std::vector<int> const& lines) override;
};

#endif /* OBJECTS_BODY_PARTS_TORSO_H_ */
