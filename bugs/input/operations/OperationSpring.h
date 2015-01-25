/*
 * OperationSpring.h
 *
 *  Created on: Nov 13, 2014
 *      Author: bogdan
 */

#ifndef INPUT_OPERATIONS_OPERATIONSPRING_H_
#define INPUT_OPERATIONS_OPERATIONSPRING_H_

#include "IOperation.h"

class PhysicsBody;
class b2MouseJoint;
class b2Body;

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
	const OperationContext* pContext;
	InputEvent::MOUSE_BUTTON boundButton;
	bool isActive;
	b2MouseJoint* mouseJoint;
	b2Body* mouseBody;
	b2Body* pressedObj;
	int onDestroySubscription;

	void onOtherBodyDestroyed(PhysicsBody* body);
	void releaseJoint();
};

#endif /* INPUT_OPERATIONS_OPERATIONSPRING_H_ */
