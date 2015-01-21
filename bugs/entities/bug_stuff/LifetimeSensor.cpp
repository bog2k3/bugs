/*
 * LifetimeSensor.cpp
 *
 *  Created on: Jan 17, 2015
 *      Author: bog
 */

#include "LifetimeSensor.h"
#include "../../neuralnet/OutputSocket.h"

LifetimeSensor::LifetimeSensor()
	: time_(0)
	, socket(std::make_shared<OutputSocket>()) {
}

void LifetimeSensor::update(float dt) {
	time_ += dt;
	socket->push_value(time_);
}
