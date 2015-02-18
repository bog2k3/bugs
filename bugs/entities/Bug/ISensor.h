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

	unsigned geneticAge = 0;
};



#endif /* ENTITIES_ISENSOR_H_ */
