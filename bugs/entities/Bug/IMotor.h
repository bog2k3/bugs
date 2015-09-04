/*
 * Motor.h
 *
 *  Created on: Feb 19, 2015
 *      Author: bog
 */

#ifndef ENTITIES_BUG_IMOTOR_H_
#define ENTITIES_BUG_IMOTOR_H_

#include <memory>
#include "../../neuralnet/InputSocket.h"

class IMotor {
public:
	virtual ~IMotor() {}

	// TODO: is shared_ptr necessary?
	// returns the input socket of the motor
	virtual std::shared_ptr<InputSocket> getInputSocket() const = 0;

	// returns the Virtual Matching Space coordinate of this sensor's nerve
	virtual float getInputVMSCoord() const = 0;
};

#endif /* ENTITIES_BUG_IMOTOR_H_ */
