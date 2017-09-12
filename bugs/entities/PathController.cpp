/*
 * PathController.cpp
 *
 *  Created on: May 21, 2017
 *      Author: bogdan
 */

#include "PathController.h"
//#include "../physics/DynamicBody.h"

PathController::PathController(physics::DynamicBody *body)
	: body_(body) {
}

PathController::~PathController() {

}

void PathController::addVertex(Vertex v) {
	lerper_.appendNode(v);
}

void PathController::addRedirect(int index) {
	lerper_.appendRedirect(index);
}

void PathController::start(float speed) {
	lerper_.start(speed, 0);
}

void PathController::update(float dt) {
	lerper_.update(dt);
	//body_->setPosition(lerper_.value().position, physics::DynamicBody::TransformSpace::Parent);
	//body_->setOrientation(lerper_.value().orientation, physics::DynamicBody::TransformSpace::Parent);
	//body_->setScale(lerper_.value().scale);
}
