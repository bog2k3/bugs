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
}


float Nose::getOutputVMSCoord(unsigned index) const {
}


void Nose::commit() {
}


void Nose::die() {
}


void Nose::onAddedToParent() {
}

