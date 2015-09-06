/*
 * LifetimeSensor.h
 *
 *  Created on: Jan 17, 2015
 *      Author: bog
 */

#ifndef ENTITIES_LIFETIMESENSOR_H_
#define ENTITIES_LIFETIMESENSOR_H_

#include "../Bug/ISensor.h"

class LifetimeSensor: public ISensor {
public:
	LifetimeSensor(float defaultVMSCoord);
	virtual ~LifetimeSensor();

	void update(float dt);

	// ISensor::
	unsigned getOutputCount() const override { return 1; }
	OutputSocket* getOutSocket(unsigned index) const override { return index==0 ? socket_ : nullptr; }
	float getOutputVMSCoord(unsigned index) const override { return index==0 ? defaultVMSCoord_ : 0; }
protected:
	float time_;
	OutputSocket* socket_;
	float defaultVMSCoord_;
};

#endif /* ENTITIES_LIFETIMESENSOR_H_ */
