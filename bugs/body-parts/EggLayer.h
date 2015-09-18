/*
 * EggLayer.h
 *
 *  Created on: Feb 10, 2015
 *      Author: bogdan
 */

#ifndef BODY_PARTS_EGGLAYER_H_
#define BODY_PARTS_EGGLAYER_H_

#include "BodyPart.h"
#include "../entities/Bug/IMotor.h"

struct EggLayerInitializationData : public BodyPartInitializationData {
	virtual ~EggLayerInitializationData() noexcept = default;
	EggLayerInitializationData();

	CummulativeValue ejectSpeed;
	CummulativeValue inputVMSCoord[2];
};

class b2WeldJoint;

class EggLayer: public BodyPart, public IMotor {
public:
	EggLayer();
	virtual ~EggLayer() override;

 	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	void update(float dt);

	float getFoodRequired();
	void useFood(float food);
	inline void setTargetEggMass(float mass) { targetEggMass_ = mass; }

	float getMass_tree() override;

	// IMotor::
	unsigned getInputCount() const override { return 2; }
	InputSocket* getInputSocket(unsigned index) const override { return index < 2 ? inputs_[index] : nullptr; }
	float getInputVMSCoord(unsigned index) const override;
#ifdef DEBUG
	std::string getMotorDebugName() const override { return getDebugName(); }
#endif

protected:
	void commit() override;
	void onAddedToParent() override;
	void die() override;
	void cacheInitializationData() override;
	void checkScale();

	b2WeldJoint* pJoint = nullptr;
	std::vector<InputSocket*> inputs_;
	bool suppressGrowth_ = false;
	bool suppressRelease_ = false;
	float initialSize_ = 0;
	float eggMassBuffer_ = 0;
	float targetEggMass_;
	float ejectSpeed_;
};

#endif /* BODY_PARTS_EGGLAYER_H_ */
