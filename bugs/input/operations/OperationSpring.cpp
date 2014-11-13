/*
 * OperationSpring.cpp
 *
 *  Created on: Nov 13, 2014
 *      Author: bogdan
 */

#include "OperationSpring.h"
#include "OperationContext.h"
#include "../../renderOpenGL/Viewport.h"
#include "../InputEvent.h"
#include "../../objects/MouseObject.h"

OperationSpring::OperationSpring(MouseObject* mObj, InputEvent::MOUSE_BUTTON boundButton)
	: mouse(mObj), pContext(nullptr), boundButton(boundButton)
{
}

OperationSpring::~OperationSpring() {
}

void OperationSpring::enter(const OperationContext* pContext) {
	this->pContext = pContext;
}

void OperationSpring::leave() {
	pContext = nullptr;
}

void OperationSpring::activate() {
}

void OperationSpring::deactivate() {
}

void OperationSpring::handleInputEvent(InputEvent& ev) {
	switch (ev.type) {
	case InputEvent::EV_MOUSE_MOVED: {
		mouse->teleport(pContext->pViewport->project(glm::vec2(ev.x, ev.y)));
		break;
	}
	default:
		break;
	}
}

void OperationSpring::update(float dt) {
}
