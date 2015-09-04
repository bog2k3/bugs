/*
 * ISensor.h
 *
 *  Created on: Dec 1, 2014
 *      Author: bog
 */

#ifndef ENTITIES_ISENSOR_H_
#define ENTITIES_ISENSOR_H_

#include <memory>

class OutputSocket;

class ISensor {
public:
	virtual ~ISensor() {}

	/**
	 * returns the sensor's OutputSocket
	 */
	virtual std::shared_ptr<OutputSocket> getOutSocket() const = 0;

	// returns the Virtual Matching Space coordinate of this sensor's nerve
	virtual float getVMSCoord() = 0;
};



#endif /* ENTITIES_ISENSOR_H_ */
