/*
 * OperationSpring.h
 *
 *  Created on: Nov 13, 2014
 *      Author: bogdan
 */

#ifndef INPUT_OPERATIONS_OPERATIONSPRING_H_
#define INPUT_OPERATIONS_OPERATIONSPRING_H_

#include "IOperation.h"

class WorldObject;

class OperationSpring: public IOperation {
public:
	OperationSpring(InputEvent::MOUSE_BUTTON boundButton);
	virtual ~OperationSpring();

	virtual void enter(const OperationContext* pContext);
	virtual void leave();
	virtual void getFocus();
	virtual void loseFocus();
	virtual void handleInputEvent(InputEvent& ev);
	virtual void update(float dt);

protected:
	WorldObject* springObj;
	const OperationContext* pContext;
	InputEvent::MOUSE_BUTTON boundButton;
	bool isActive;
};

#endif /* INPUT_OPERATIONS_OPERATIONSPRING_H_ */
