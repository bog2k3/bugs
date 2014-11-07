/*
 * IOperation.h
 *
 *  Created on: Nov 4, 2014
 *      Author: bog
 */

#ifndef INPUT_IOPERATION_H_
#define INPUT_IOPERATION_H_

class InputEvent;

class IOperation {
public:
	virtual ~IOperation() {
	}

	virtual void handleInput(InputEvent& ev) = 0;
	virtual void update(float dt) {}
};

#endif /* INPUT_IOPERATION_H_ */
