/*
 * IMotor.h
 *
 *  Created on: Dec 1, 2014
 *      Author: bog
 */

#ifndef ENTITIES_IMOTOR_H_
#define ENTITIES_IMOTOR_H_

class IMotor {
public:
	virtual ~IMotor() {}

	/**
	 * command the motor with the given intensity;
	 * intensity depends on the type and properties of the motor
	 */
	virtual void action(float intensity) = 0;
};

#endif /* ENTITIES_IMOTOR_H_ */
