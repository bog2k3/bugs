/*
 * LifetimeSensor.cpp
 *
 *  Created on: Jan 17, 2015
 *      Author: bog
 */

#include "../Bug/LifetimeSensor.h"

#include "../../neuralnet/OutputSocket.h"

LifetimeSensor::LifetimeSensor(float defaultVMSCoord)
	: time_(0)
	, socket_(new OutputSocket())
	, defaultVMSCoord_(defaultVMSCoord) {
}

LifetimeSensor::~LifetimeSensor() {
	delete socket_;
}

void LifetimeSensor::update(float dt) {
	time_ += dt;
	socket_->push_value(time_);
}
