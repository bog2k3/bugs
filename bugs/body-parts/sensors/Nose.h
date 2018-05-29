/*
 * Nose.h
 *
 *  Created on: Jan 1, 2016
 *      Author: bog
 */

#ifndef BODY_PARTS_SENSORS_NOSE_H_
#define BODY_PARTS_SENSORS_NOSE_H_

#include "../BodyPart.h"
#include "../../entities/Bug/ISensor.h"
#include "../../entities/enttypes.h"

#include <glm/vec2.hpp>

static constexpr EntityType NoseDetectableFlavours[] {
	EntityType::	FOOD_CHUNK,
	EntityType::	BUG,
};
static constexpr size_t NoseDetectableFlavoursCount = sizeof(NoseDetectableFlavours)/sizeof(NoseDetectableFlavours[0]);

class Nose : public BodyPart, public ISensor {
public:
	Nose(BodyPartContext const& context, BodyCell& cell);
	~Nose() override;

	void draw(Viewport* vp) override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;

	void update(float dt);

	// ISensor::
	unsigned getOutputCount() const override { return NoseDetectableFlavoursCount; }
	OutputSocket* getOutputSocket(unsigned index) const override { return index<NoseDetectableFlavoursCount ? outputSocket_[index] : nullptr; }
	float getOutputVMSCoord(unsigned index) const override { return index > NoseDetectableFlavoursCount ? 0 : outputVMSCoord_[index]; }

	static float getDensity(BodyCell const& cell);
	static float getRadius(BodyCell const& cell, float angle);

protected:
	OutputSocket* outputSocket_[NoseDetectableFlavoursCount];
	float outputVMSCoord_[NoseDetectableFlavoursCount];

	void updateFixtures() override;
	//void onAddedToParent() override;
};


#endif /* BODY_PARTS_SENSORS_NOSE_H_ */
