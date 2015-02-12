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

	virtual int getNumberOfInputs() = 0;
	virtual std::shared_ptr<InputSocket> getInputSocket(int index) = 0;
};

#endif /* ENTITIES_IMOTOR_H_ */
