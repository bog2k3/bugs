/*
 * OperationGui.cpp
 *
 *  Created on: Mar 28, 2015
 *      Author: bog
 */

#include "OperationGui.h"
#include "../../GUI/GuiSystem.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

OperationGui::OperationGui(GuiSystem &guiSystem)
	: pGuiSystem(&guiSystem) {

}

void OperationGui::enter(const OperationContext* pContext) {

}

void OperationGui::leave() {

}

void OperationGui::getFocus() {

}

void OperationGui::loseFocus() {

}

void OperationGui::handleInputEvent(InputEvent& ev) {
	pGuiSystem->handleInput(ev);
}

void OperationGui::update(float dt) {

}
