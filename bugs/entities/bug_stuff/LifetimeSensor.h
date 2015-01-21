/*
 * LifetimeSensor.h
 *
 *  Created on: Jan 17, 2015
 *      Author: bog
 */

#ifndef ENTITIES_LIFETIMESENSOR_H_
#define ENTITIES_LIFETIMESENSOR_H_

#include "ISensor.h"

class LifetimeSensor: public ISensor {
public:
	LifetimeSensor();
	virtual ~LifetimeSensor() {}

	void update(float dt);

	std::shared_ptr<OutputSocket> getOutSocket() const override { return socket; }
protected:
	float time_;
	std::shared_ptr<OutputSocket> socket;
};

#endif /* ENTITIES_LIFETIMESENSOR_H_ */
