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

protected:
	void commit() override;
	void onAddedToParent() override;
	void die() override;

	b2WeldJoint* pJoint;
	std::vector<std::shared_ptr<InputSocket>> inputs_;

public:
	decltype(inputs_) const& getInputSockets() const { return inputs_; }
};

#endif /* BODY_PARTS_EGGLAYER_H_ */
