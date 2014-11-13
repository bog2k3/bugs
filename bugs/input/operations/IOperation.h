#pragma once

#include "../InputEvent.h"

class OperationsStack;
class OperationContext;

class IOperation
{
public:
	virtual ~IOperation() { }

	// called once after the operation is pushed onto the stack
	// a context is passed in this function, containing all necessary data for the operation
	virtual void enter(const OperationContext* pContext)=0;

	// called once when this operation is about to be removed from the stack
	virtual void leave()=0;

	// called when the operation has got focus (top of stack)
	virtual void getFocus()=0;

	// called before operation loses focus (another operation is pushed onto the stack on top of this)
	virtual void loseFocus()=0;

	virtual void handleInputEvent(InputEvent &event)=0;

	// this is invoked when a command from a pop-up menu or toolbar is received that has not been
	// handled by the application layer; return true if the event was handled or false
	// to pass it through to the next operation on the stack
	virtual bool command(int cmd, long param) { return false; }

	// this is called only when the operation is active, on a per-frame basis
	virtual void update(float dt) {}
};
