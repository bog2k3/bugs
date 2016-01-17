/*
 * Nose.cpp
 *
 *  Created on: Jan 1, 2016
 *      Author: bog
 */

#include "Nose.h"
#include "../../neuralnet/OutputSocket.h"

const glm::vec3 debug_color(1.f, 0.8f, 0.f);

#define DEBUG_DRAW_GRIPPER

Nose::Nose()
	: BodyPart(BodyPartType::GRIPPER, std::make_shared<NoseInitializationData>())
	, outputSocket_(new OutputSocket())
{
}

Nose::~Nose() {
}

void Nose::draw(RenderContext const& ctx) {
}


glm::vec2 Nose::getChildAttachmentPoint(float relativeAngle) {
}


void Nose::update(float dt) {
	/*
	 * detect the nearest object of the right flavour and compute the output signal like this:
	 * s0 = 1/(d^2+1) * max(0, cos(phi))
	 * 		where:  > [d] is the distance from sensor to object
	 * 				> [phi] is the angle of the object relative to the sensor's orientation in space
	 * n = 25% * s0 	this is noise which is proportional to the strength of the signal, in such a way as to always prevent perfect accuracy
	 * s = n + s0;	// the output signal which is 80% useful data and 20% noise (precision is 80%)
	 */
}


float Nose::getOutputVMSCoord(unsigned index) const {
}


void Nose::commit() {
}


void Nose::die() {
}


void Nose::onAddedToParent() {
}

