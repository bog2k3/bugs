/*
 * Eye.h
 *
 *  Created on: Jul 19, 2018
 *      Author: bog
 */

#ifndef BUGS_BODY_PARTS_SENSORS_EYE_H_
#define BUGS_BODY_PARTS_SENSORS_EYE_H_

#include "../BodyPart.h"
#include "../ISensor.h"
#include "../../neuralnet/OutputSocket.h"

class Eye: public BodyPart, public ISensor  {
public:
	Eye(BodyPartContext const& context, BodyCell& cell);
	virtual ~Eye() override;

	void draw(Viewport* vp) override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;

	void update(float dt);

	// ISensor::
	unsigned getOutputCount() const override { return 1; }
	// TODO figure out how this sensor should work and how many outputs it should have
	OutputSocket* getOutputSocket(unsigned index) const override { return index == 0 ? const_cast<OutputSocket*>(&outputSocket_) : nullptr; }
	float getOutputVMSCoord(unsigned index) const override { return index == 0 ? 0 : outputVMSCoord_; }

	static float getDensity(BodyCell const& cell);
	static float getRadius(BodyCell const& cell, float angle);

protected:
	OutputSocket outputSocket_;
	float outputVMSCoord_;

	void updateFixtures() override;
};

#endif /* BUGS_BODY_PARTS_SENSORS_EYE_H_ */
