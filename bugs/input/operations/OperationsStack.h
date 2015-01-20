#pragma once

#include <vector>
#include <memory>

class Viewport;
class OperationContext;
class IOperation;
class InputEvent;
class IOperationSpatialLocator;
class b2World;

class OperationsStack
{
public:
	OperationsStack(Viewport* pViewport, IOperationSpatialLocator* locator, b2World* physics);
	~OperationsStack();

	// pushes a new operation onto the stack; the new operation will become active
	void pushOperation(std::unique_ptr<IOperation> pOperation);
	// swaps the top operation with a new one; the old one will be deleted
	void swapTopOperation(std::unique_ptr<IOperation> pNewOperation);
	// returns a pointer to the top operation
	inline IOperation* getTopOperation() { return m_stack.back().get(); }
	// removes and deletes the top operation; the operation below will become active
	void removeTopOperation();

	void handleInputEvent(InputEvent &ev);
	bool command(int cmd, long param);
	void update(float dt);

protected:
	OperationContext *m_context;

	std::vector<std::unique_ptr<IOperation>> m_stack;
};
