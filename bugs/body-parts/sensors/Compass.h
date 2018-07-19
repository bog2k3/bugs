/*
 * Compass.h
 *
 *  Created on: Jul 19, 2018
 *      Author: bog
 */

#ifndef BUGS_BODY_PARTS_SENSORS_COMPASS_H_
#define BUGS_BODY_PARTS_SENSORS_COMPASS_H_

#include "../BodyPart.h"
#include "../ISensor.h"
#include "../../neuralnet/OutputSocket.h"

class Compass: public BodyPart, public ISensor  {
public:
	Compass(BodyPartContext const& context, BodyCell& cell);
	virtual ~Compass() override;

	void draw(Viewport* vp) override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;

	void update(float dt);

	// ISensor::
	unsigned getOutputCount() const override { return 1; }
	OutputSocket* getOutputSocket(unsigned index) const override { return index == 0 ? const_cast<OutputSocket*>(&outputSocket_) : nullptr; }
	float getOutputVMSCoord(unsigned index) const override { return index == 0 ? 0 : outputVMSCoord_; }

	static float getDensity(BodyCell const& cell);
	static float getRadius(BodyCell const& cell, float angle);

protected:
	OutputSocket outputSocket_;
	float outputVMSCoord_;

	void updateFixtures() override;
};

#endif /* BUGS_BODY_PARTS_SENSORS_COMPASS_H_ */
