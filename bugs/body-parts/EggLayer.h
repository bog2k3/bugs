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
#include "../neuralnet/InputSocket.h"

class b2WeldJoint;

#define DEBUG_DRAW_EGGLAYER

class EggLayer: public BodyPart, public IMotor {
public:
	EggLayer();
	virtual ~EggLayer() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	void update(float dt);

	unsigned getNumberOfInputs() override { return inputs_.size(); }
	std::shared_ptr<InputSocket> getInputSocket(unsigned index) override { assert(index < inputs_.size()); return inputs_[index]; }

protected:
	void commit() override;
	void onAddedToParent() override;

	b2WeldJoint* pJoint;
	std::vector<std::shared_ptr<InputSocket>> inputs_;
};

#endif /* BODY_PARTS_EGGLAYER_H_ */
