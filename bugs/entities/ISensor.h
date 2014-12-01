/*
 * ISensor.h
 *
 *  Created on: Dec 1, 2014
 *      Author: bog
 */

#ifndef ENTITIES_ISENSOR_H_
#define ENTITIES_ISENSOR_H_

class OutputSocket;

class ISensor {
public:
	virtual void ~ISensor() {}

	/**
	 * frame update
	 */
	virtual void update() = 0;

	/**
	 * return the number of available outputs from this sensor
	 * (the number is constant for this type of sensor)
	 */
	virtual int getOutputCount() const = 0;

	/**
	 * returns an array of OutputSocket*
	 */
	virtual OutputSocket** getOutSockets() const = 0;
};



#endif /* ENTITIES_ISENSOR_H_ */
