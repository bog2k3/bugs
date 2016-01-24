/*
 * Nose.cpp
 *
 *  Created on: Jan 1, 2016
 *      Author: bog
 */

#include "Nose.h"
#include "../BodyConst.h"
#include "../../neuralnet/OutputSocket.h"
#include "../../math/math2D.h"
#include "../../utils/UpdateList.h"
#include <glm/gtx/rotate_vector.hpp>

const glm::vec3 debug_color(1.f, 0.8f, 0.f);

#define DEBUG_DRAW_GRIPPER

Nose::Nose()
	: BodyPart(BodyPartType::SENSOR_PROXIMITY, std::make_shared<NoseInitializationData>())
{
	for (uint i=0; i<getOutputCount(); i++)
		outputSocket_[i] = new OutputSocket();
}

Nose::~Nose() {
	for (uint i=0; i<getOutputCount(); i++)
		delete outputSocket_[i];
}

void Nose::draw(RenderContext const& ctx) {
}


glm::vec2 Nose::getChildAttachmentPoint(float relativeAngle) {
	if (!geneValuesCached_) {
		cacheInitializationData();
	}
	glm::vec2 ret(glm::rotate(glm::vec2(sqrtf(size_ * PI_INV), 0), relativeAngle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
}


void Nose::update(float dt) {
	/*
	 * max radius & accuracy are proportional to the size of the nose
	 * detect the nearest object of the right flavour and compute the output signal like this:
	 * s0 = 1/(d^2+1) * max(0, cos(phi))
	 * 		where:  > [d] is the distance from sensor to object
	 * 				> [phi] is the angle of the object relative to the sensor's orientation in space
	 * n = 25% * s0 	this is noise which is proportional to the strength of the signal, in such a way as to always prevent perfect accuracy
	 * s = n + s0;	// the output signal which is 80% useful data and 20% noise (precision is 80%)
	 */
}


float Nose::getOutputVMSCoord(unsigned index) const {
	if (index >= getOutputCount())
		return 0;
	auto initData = std::dynamic_pointer_cast<NoseInitializationData>(getInitializationData());
	if (initData)
		return initData->outputVMSCoord[index].clamp(0, BodyConst::MaxVMSCoordinateValue);
	else
		return 0;
}


void Nose::commit() {
}


void Nose::die() {
	if (getUpdateList())
		getUpdateList()->remove(this);
	physBody_.categoryFlags_ |= EventCategoryFlags::FOOD;
}


void Nose::onAddedToParent() {
	assertDbg(getUpdateList() && "update list should be available to the body at this time");
	getUpdateList()->add(this);
}

