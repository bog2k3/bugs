#include "OperationsStack.h"
#include "OperationContext.h"
#include "IOperation.h"
#include "../GLFWInput.h"
#include <assert.h>

OperationsStack::OperationsStack(Viewport* pViewport, IOperationSpatialLocator* locator, b2World* physics)
	: m_context(new OperationContext(pViewport, this, locator, physics))
	, m_stack()
{
	GLFWInput::onInputEvent.add(std::bind(&OperationsStack::handleInputEvent, this, std::placeholders::_1));
}

OperationsStack::~OperationsStack()
{
	while (m_stack.size() > 0)
		removeTopOperation();
	delete m_context;
}

void OperationsStack::pushOperation(std::unique_ptr<IOperation> pOperation)
{
	if (m_stack.size() > 0)
		m_stack.back()->loseFocus();

	IOperation* ptrOp = pOperation.get();
	m_stack.push_back(std::move(pOperation));
	ptrOp->enter(m_context);

	if (ptrOp == m_stack.back().get()) // if the operation pushed another one during enter, skip activate
		ptrOp->getFocus();
}

void OperationsStack::swapTopOperation(std::unique_ptr<IOperation> pNewOperation)
{
	assert(m_stack.size() > 0);
	m_stack.back()->loseFocus();
	m_stack.back()->leave();

	IOperation* ptrOp = pNewOperation.get();
	m_stack.back() = std::move(pNewOperation);
	ptrOp->enter(m_context);

	if (ptrOp == m_stack.back().get()) // if the operation pushed another one during enter, skip activate
		ptrOp->getFocus();
}

void OperationsStack::removeTopOperation()
{
	assert(m_stack.size() > 0);
	m_stack.back()->loseFocus();
	m_stack.back()->leave();
	m_stack.pop_back();

	if (m_stack.size() > 0)
		m_stack.back()->getFocus();
}

void OperationsStack::handleInputEvent(InputEvent &ev)
{
	int i=m_stack.size()-1;
	while (!ev.isConsumed() && i>=0) {
		m_stack[i--]->handleInputEvent(ev);
	}
}
bool OperationsStack::command(int cmd, long param)
{
	int i=m_stack.size()-1;
	bool handled = false;
	while (!handled && i>=0) {
		handled = m_stack[i--]->command(cmd, param);
	}
	return handled;
}
void OperationsStack::update(float dt) {
	for (unsigned i=0; i<m_stack.size(); i++)
		m_stack[i]->update(dt);
}
