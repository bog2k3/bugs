/*
 * IMotor.h
 *
 *  Created on: Dec 1, 2014
 *      Author: bog
 */

#ifndef ENTITIES_IMOTOR_H_
#define ENTITIES_IMOTOR_H_

class InputSocket;
#include <memory>

class IMotor {
public:
	virtual ~IMotor() {}

	virtual unsigned getNumberOfInputs() = 0;
	virtual std::shared_ptr<InputSocket> getInputSocket(unsigned index) = 0;

	unsigned geneticAge = 0;
};

#endif /* ENTITIES_IMOTOR_H_ */
