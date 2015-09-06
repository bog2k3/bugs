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
	virtual ~ISensor() {}

	// return the number of outputs this sensor generates
	virtual unsigned getOutputCount() const = 0;

	// returns one of this sensor's OutputSockets
	virtual OutputSocket* getOutSocket(unsigned index) const = 0;

	// returns the Virtual Matching Space coordinate of one of this sensor's nerves
	virtual float getOutputVMSCoord(unsigned index) const = 0;
};



#endif /* ENTITIES_ISENSOR_H_ */
