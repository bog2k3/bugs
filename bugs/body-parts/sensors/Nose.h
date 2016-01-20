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
#include <memory>

struct NoseInitializationData : public BodyPartInitializationData {
	virtual ~NoseInitializationData() noexcept = default;
	NoseInitializationData() = default;

	CummulativeValue outputVMSCoord; // output nerve VMS coordinate
};


class Nose : public BodyPart, public ISensor {
public:
	Nose();
	~Nose() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	void update(float dt);

	// ISensor::
	unsigned getOutputCount() const override { return 1; }
	OutputSocket* getOutputSocket(unsigned index) const override { return index==0 ? outputSocket_ : 0; }
	float getOutputVMSCoord(unsigned index) const override;

protected:
	OutputSocket* outputSocket_;

	void commit() override;
	void die() override;
	void onAddedToParent() override;
};


#endif /* BODY_PARTS_SENSORS_NOSE_H_ */