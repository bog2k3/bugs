/*
 * EggLayer.h
 *
 *  Created on: Feb 10, 2015
 *      Author: bogdan
 */

#ifndef BODY_PARTS_EGGLAYER_H_
#define BODY_PARTS_EGGLAYER_H_

#include "BodyPart.h"
#include "../neuralnet/InputSocket.h"

class b2WeldJoint;

class EggLayer: public BodyPart {
public:
	EggLayer();
	virtual ~EggLayer() override;

 	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	void update(float dt);

	float getFoodRequired();
	void useFood(float food);
	inline void setTargetEggMass(float mass) { targetEggMass_ = mass; }

protected:
	void commit() override;
	void onAddedToParent() override;
	void die() override;
	void checkScale();

	b2WeldJoint* pJoint;
	std::vector<std::shared_ptr<InputSocket>> inputs_;
	bool suppressGrowth_;
	bool suppressRelease_;
	float eggMassBuffer_;
	float targetEggMass_;
	float ejectSpeed_;	// TODO put this in intialization data to be changed by genes

public:
	decltype(inputs_) const& getInputSockets() const { return inputs_; }
};

#endif /* BODY_PARTS_EGGLAYER_H_ */
