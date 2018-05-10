/*
 * OperationBreakJoint.h
 *
 *  Created on: May 10, 2018
 *      Author: bogdan
 */

#ifndef INPUT_OPERATIONS_OperationBreakJoint_H_
#define INPUT_OPERATIONS_OperationBreakJoint_H_

#include <boglfw/input/operations/IOperation.h>

class OperationBreakJoint: public IOperation {
public:
	OperationBreakJoint(InputEvent::MOUSE_BUTTON boundButton);
	virtual ~OperationBreakJoint();

	virtual void enter(const OperationContext* pContext);
	virtual void leave();
	virtual void getFocus();
	virtual void loseFocus();
	virtual void handleInputEvent(InputEvent& ev);

protected:
	const OperationContext* pContext;
	InputEvent::MOUSE_BUTTON boundButton;
};

#endif /* INPUT_OPERATIONS_OperationBreakJoint_H_ */
