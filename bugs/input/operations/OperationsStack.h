#pragma once

#include "OperationContext.h"

class Viewport;

class IOperation;
class BSPTree;

class OperationsStack
{
public:
	OperationsStack(OperationContext &context);
	~OperationsStack();

	// pushes a new operation onto the stack; the new operation will become active
	void pushOperation(IOperation *pOperation);
	// swaps the top operation with a new one; the old one will be deleted
	void swapTopOperation(IOperation *pNewOperation);
	// returns a pointer to the top operation
	inline IOperation* getTopOperation() { return m_stack.back(); }
	// removes and deletes the top operation; the operation below will become active
	void removeTopOperation();

	bool mouseWheel(int x, int y, float zDelta, int fwKeys);
	bool mouseMoved( int x, int y, int deltaX, int deltaY, bool leftBtnDown, bool rightBtnDown, int fwKeys );
	bool mouseDown(MouseButton button, int x, int y, int fwKeys);
	bool mouseUp(MouseButton button, int x, int y, int fwKeys);
	bool keyDown(int vKeyCode);
	bool keyUp(int vKeyCode);
	bool command(int cmd, LPARAM lParam);

protected:
	OperationContext m_context;

	vector<IOperation*> m_stack;
};
