/*
 * EggLayer.cpp
 *
 *  Created on: Feb 10, 2015
 *      Author: bogdan
 */

#include "EggLayer.h"

EggLayer::EggLayer()
	: BodyPart(BODY_PART_EGGLAYER, std::make_shared<BodyPartInitializationData>())
{
}

EggLayer::~EggLayer() {
	// TODO Auto-generated destructor stub
}

void EggLayer::draw(RenderContext const& ctx) {

}

glm::vec2 EggLayer::getChildAttachmentPoint(float relativeAngle) {

}

void EggLayer::update(float dt){

}

void EggLayer::commit() {

}

void EggLayer::onAddedToParent() {

}
