/*
 * OperationBreakJoint.cpp
 *
 *  Created on: May 10, 2018
 *      Author: bogdan
 */

#include "OperationBreakJoint.h"
#include "ObjectTypesAndFlags.h"
#include "body-parts/BodyPart.h"

#include <boglfw/input/operations/OperationContext.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/input/InputEvent.h>
#include <boglfw/input/operations/IOperationSpatialLocator.h>
#include <boglfw/math/box2glm.h>
#include <boglfw/physics/PhysicsBody.h>

#include <Box2D/Box2D.h>
#include <glm/vec2.hpp>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

OperationBreakJoint::OperationBreakJoint(InputEvent::MOUSE_BUTTON boundButton)
	: pContext(nullptr), boundButton(boundButton)
{
}

OperationBreakJoint::~OperationBreakJoint() {
}

void OperationBreakJoint::enter(const OperationContext* pContext) {
	this->pContext = pContext;
}

void OperationBreakJoint::leave() {
	pContext = nullptr;
}

void OperationBreakJoint::getFocus() {
}

void OperationBreakJoint::loseFocus() {
}

void OperationBreakJoint::handleInputEvent(InputEvent& ev) {
	switch (ev.type) {
	case InputEvent::EV_MOUSE_DOWN: {
		if (ev.mouseButton != boundButton)
			break;
		glm::vec2 wldClickPos = pContext->pViewport->unproject(glm::vec3(ev.x, ev.y, 0));
		auto pressedObj = pContext->locator->getBodyAtPos(wldClickPos);
		if (pressedObj == nullptr)
			return;
		PhysicsBody* phPtr = (PhysicsBody*)pressedObj->GetUserData();
		switch (phPtr->userObjectType_) {
		case ObjectTypes::BPART_BONE:
		case ObjectTypes::BPART_EGGLAYER:
		case ObjectTypes::BPART_FATCELL:
		case ObjectTypes::BPART_GRIPPER:
		case ObjectTypes::BPART_MOUTH:
		case ObjectTypes::BPART_MUSCLE:
		case ObjectTypes::BPART_NOSE:
		{
			BodyPart* p = static_cast<BodyPart*>(phPtr->userPointer_);
			p->disconnectAllNeighbors();
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}
