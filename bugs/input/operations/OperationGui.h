/*
 * OperationGui.h
 *
 *  Created on: Mar 28, 2015
 *      Author: bog
 */

#ifndef INPUT_OPERATIONS_OPERATIONGUI_H_
#define INPUT_OPERATIONS_OPERATIONGUI_H_

#include "IOperation.h"

class GuiSystem;

class OperationGui: public IOperation {
public:
	OperationGui(GuiSystem &guiSystem);

	void enter(const OperationContext* pContext) override;
	void leave() override;
	void getFocus() override;
	void loseFocus() override;
	void handleInputEvent(InputEvent& ev) override;
	void update(float dt) override;

private:
	GuiSystem *pGuiSystem = nullptr;
};

#endif /* INPUT_OPERATIONS_OPERATIONGUI_H_ */
