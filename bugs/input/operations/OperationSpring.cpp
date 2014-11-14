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
	: mouse(mObj), pContext(nullptr), boundButton(boundButton), isActive(false)
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

void OperationSpring::getFocus() {
}

void OperationSpring::loseFocus() {
}

void OperationSpring::handleInputEvent(InputEvent& ev) {
	switch (ev.type) {
	case InputEvent::EV_MOUSE_DOWN: {
		if (ev.mouseButton != boundButton)
			break;
		isActive = true;
		/*Spring s(
				AttachPoint(b.getRigidBody(),
					glm::vec2(0.5f, 0.15f)
				),
				AttachPoint(&mouse, glm::vec2(0)),
				10.f, // k
				0.1f // initialLength
				);
		wld.addObject(new WorldObject(&s));*/
		break;
	}
	case InputEvent::EV_MOUSE_MOVED: {
		mouse->teleport(pContext->pViewport->unproject(glm::vec2(ev.x, ev.y)));
		break;
	}
	default:
		break;
	}
}

void OperationSpring::update(float dt) {
}
