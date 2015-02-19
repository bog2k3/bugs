/*
 * Motor.h
 *
 *  Created on: Feb 19, 2015
 *      Author: bog
 */

#ifndef ENTITIES_BUG_MOTOR_H_
#define ENTITIES_BUG_MOTOR_H_

#include <memory>
#include "../../neuralnet/InputSocket.h"

struct Motor {
	std::shared_ptr<InputSocket> inputSocket;
	int geneticAge = 0;

	Motor(decltype(inputSocket) inputSocket, int geneticAge)
		: inputSocket(inputSocket), geneticAge(geneticAge) {
	}
};

#endif /* ENTITIES_BUG_MOTOR_H_ */
