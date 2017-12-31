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


class EggLayer: public BodyPart, public IMotor {
public:
	EggLayer(BodyPartContext const& context, BodyCell& cell);
	virtual ~EggLayer() override;

 	void draw(RenderContext const& ctx) override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;
	void update(float dt);

	float getFoodRequired();
	void useFood(float food);
	inline void setTargetEggMass(float mass) { targetEggMass_ = mass; }

	// IMotor::
	unsigned getInputCount() const override { return 2; }
	InputSocket* getInputSocket(unsigned index) const override { return index < 2 ? inputs_[index] : nullptr; }
	float getInputVMSCoord(unsigned index) const override { return index < 2 ? VMSCoords_[index] : 0; }
#ifdef DEBUG
	std::string getMotorDebugName() const override { return getDebugName(); }
#endif

	static float getDensity(BodyCell const& cell);

protected:
	void updateFixtures() override;
	//void onAddedToParent() override;
	void die() override;
	//void cacheInitializationData() override;

//	b2WeldJoint* pJoint = nullptr;
	InputSocket* inputs_[2];
	float VMSCoords_[2];
	bool suppressGrowth_ = false;
	bool suppressRelease_ = false;
	float initialSize_ = 0;
	float eggMassBuffer_ = 0;
	float targetEggMass_;
	float ejectSpeed_;
};

#endif /* BODY_PARTS_EGGLAYER_H_ */
