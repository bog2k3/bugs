/*
 * Motor.h
 *
 *  Created on: Feb 19, 2015
 *      Author: bog
 */

#ifndef ENTITIES_BUG_IMOTOR_H_
#define ENTITIES_BUG_IMOTOR_H_

class InputSocket;

class IMotor {
public:
	virtual ~IMotor() {}

	// return the number of inputs this motor takes
	virtual unsigned getInputCount() const = 0;

	// returns one input socket of the motor
	virtual InputSocket* getInputSocket(unsigned index) const = 0;

	// returns the Virtual Matching Space coordinate of one of this motor's input nerve
	virtual float getInputVMSCoord(unsigned index) const = 0;
};

#endif /* ENTITIES_BUG_IMOTOR_H_ */
