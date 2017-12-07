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
#include <memory>

static constexpr EntityType NoseDetectableFlavours[] {
	EntityType::	FOOD_CHUNK,
	EntityType::	BUG,
};
static constexpr size_t NoseDetectableFlavoursCount = sizeof(NoseDetectableFlavours)/sizeof(NoseDetectableFlavours[0]);

struct NoseInitializationData : public BodyPartInitializationData {
	virtual ~NoseInitializationData() noexcept = default;
	NoseInitializationData() = default;

	CummulativeValue outputVMSCoord[NoseDetectableFlavoursCount]; // output nerve VMS coordinate
};

class b2WeldJoint;

class Nose : public BodyPart, public ISensor {
public:
	Nose();
	~Nose() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getAttachmentPoint(float relativeAngle) override;

	void update(float dt);

	// ISensor::
	unsigned getOutputCount() const override { return NoseDetectableFlavoursCount; }
	OutputSocket* getOutputSocket(unsigned index) const override { return index<NoseDetectableFlavoursCount ? outputSocket_[index] : nullptr; }
	float getOutputVMSCoord(unsigned index) const override;

protected:
	OutputSocket* outputSocket_[NoseDetectableFlavoursCount];
	b2WeldJoint* pJoint = nullptr;

	void commit() override;
	void die() override;
	//void onAddedToParent() override;
};


#endif /* BODY_PARTS_SENSORS_NOSE_H_ */
