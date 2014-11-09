#include "OperationsStack.h"
#include "IOperation.h"

namespace lifeApplication
{
	OperationsStack::OperationsStack(OperationContext &context)
		: m_context(context)
		, m_stack()
	{
		m_context.pStack = this;
	}

	OperationsStack::~OperationsStack()
	{
		while (m_stack.size() > 0)
			removeTopOperation();
	}

	void OperationsStack::pushOperation(IOperation *pOperation)
	{
		if (m_stack.size() > 0)
			m_stack.back()->deactivate();

		m_stack.push_back(pOperation);
		pOperation->enter(&m_context);

		if (pOperation == m_stack.back()) // if the operation pushed another one during enter, skip activate
			pOperation->activate();
	}

	void OperationsStack::swapTopOperation(IOperation *pNewOperation)
	{
		assert(m_stack.size() > 0);
		m_stack.back()->deactivate();
		m_stack.back()->leave();
		delete m_stack.back();

		m_stack.back() = pNewOperation;
		pNewOperation->enter(&m_context);

		if (pNewOperation == m_stack.back()) // if the operation pushed another one during enter, skip activate
			pNewOperation->activate();
	}

	void OperationsStack::removeTopOperation()
	{
		assert(m_stack.size() > 0);
		m_stack.back()->deactivate();
		m_stack.back()->leave();
		delete m_stack.back();
		m_stack.pop_back();

		if (m_stack.size() > 0)
			m_stack.back()->activate();
	}

	bool OperationsStack::mouseWheel(int x, int y, float zDelta, int fwKeys)
	{
		int i=m_stack.size()-1;
		bool handled = false;
		while (!handled && i>=0) {
			handled = m_stack[i--]->mouseWheel(x,y,zDelta,fwKeys);
		}
		return handled;
	}

	bool OperationsStack::mouseMoved( int x, int y, int deltaX, int deltaY, bool leftBtnDown, bool rightBtnDown, int fwKeys )
	{
		int i=m_stack.size()-1;
		bool handled = false;
		while (!handled && i>=0) {
			handled = m_stack[i--]->mouseMoved(x,y,deltaX,deltaY,leftBtnDown,rightBtnDown,fwKeys);
		}
		return handled;
	}

	bool OperationsStack::mouseDown(MouseButton button, int x, int y, int fwKeys)
	{
		int i=m_stack.size()-1;
		bool handled = false;
		while (!handled && i>=0) {
			handled = m_stack[i--]->mouseDown(button,x,y,fwKeys);
		} 
		return handled;
	}

	bool OperationsStack::mouseUp(MouseButton button, int x, int y, int fwKeys)
	{
		int i=m_stack.size()-1;
		bool handled = false;
		while (!handled && i>=0) {
			handled = m_stack[i--]->mouseUp(button,x,y,fwKeys);
		}
		return handled;
	}

	bool OperationsStack::keyDown(int vKeyCode)
	{
		int i=m_stack.size()-1;
		bool handled = false;
		while (!handled && i>=0) {
			handled = m_stack[i--]->keyDown(vKeyCode);
		}
		return handled;
	}

	bool OperationsStack::keyUp(int vKeyCode)
	{
		int i=m_stack.size()-1;
		bool handled = false;
		while (!handled && i>=0) {
			handled = m_stack[i--]->keyUp(vKeyCode);
		}
		return handled;
	}

	bool OperationsStack::command(int cmd, LPARAM lParam)
	{
		int i=m_stack.size()-1;
		bool handled = false;
		while (!handled && i>=0) {
			handled = m_stack[i--]->command(cmd, lParam);
		}
		return handled;
	}

}
