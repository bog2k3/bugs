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
#include "../IWorldManager.h"
#include "IOperationSpatialLocator.h"
#include "../../objects/MouseObject.h"
#include "../../objects/WorldObject.h"
#include "../../physics/Spring.h"

OperationSpring::OperationSpring(MouseObject* mObj, InputEvent::MOUSE_BUTTON boundButton)
	: mouse(mObj), springObj(nullptr), pContext(nullptr), boundButton(boundButton), isActive(false)
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
		glm::vec2 wldClickPos = pContext->pViewport->unproject(glm::vec2(ev.x, ev.y));
		WorldObject* pressedObj = pContext->locator->getObjectAtPos(wldClickPos);
		if (pressedObj != nullptr) {
			Spring* s = new Spring(
				AttachPoint(pressedObj->getRigidBody(),
					pressedObj->getRigidBody()->worldToLocal(wldClickPos)
				),
				AttachPoint(mouse, glm::vec2(0)),
				10.f, // k
				0.0f // initialLength
				);
			springObj = new WorldObject(s);
			pContext->worldManager->addObject(springObj);
		}
		break;
	}
	case InputEvent::EV_MOUSE_UP: {
		if (ev.mouseButton != boundButton || !isActive)
			break;
		isActive = false;
		pContext->worldManager->removeObject(springObj);
		delete springObj;
		springObj = nullptr;
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
