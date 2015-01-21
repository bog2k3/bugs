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
#include "IOperationSpatialLocator.h"
#include "../../math/box2glm.h"
#include <Box2D/Box2D.h>

OperationSpring::OperationSpring(InputEvent::MOUSE_BUTTON boundButton)
	: pContext(nullptr), boundButton(boundButton), isActive(false), mouseJoint(nullptr), mouseBody(nullptr)
{
}

OperationSpring::~OperationSpring() {
}

void OperationSpring::enter(const OperationContext* pContext) {
	this->pContext = pContext;
}

void OperationSpring::leave() {
	if (mouseBody) {
		pContext->physics->DestroyBody(mouseBody);
		mouseBody = nullptr;
	}
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
		glm::vec2 wldClickPos = pContext->pViewport->unproject(glm::vec2(ev.x, ev.y));
		b2Body* pressedObj = pContext->locator->getBodyAtPos(wldClickPos);
		if (pressedObj != nullptr) {
			isActive = true;
			b2BodyDef bdef;
			bdef.type = b2_staticBody;
			bdef.position = g2b(wldClickPos);
			mouseBody = pContext->physics->CreateBody(&bdef);
			b2CircleShape shape;
			shape.m_radius = 0.05f;
			b2FixtureDef fdef;
			fdef.shape = &shape;
			b2Fixture* fix = mouseBody->CreateFixture(&fdef);
			b2Filter filter;
			filter.maskBits = 0;
			fix->SetFilterData(filter);
			b2MouseJointDef def;
			def.target = g2b(wldClickPos);
			def.bodyA = mouseBody;
			def.bodyB = pressedObj;
			def.bodyB->SetAwake(true);
			def.maxForce = 100;
			mouseJoint = (b2MouseJoint*)pContext->physics->CreateJoint(&def);
		}
		break;
	}
	case InputEvent::EV_MOUSE_UP: {
		if (ev.mouseButton != boundButton || !isActive)
			break;
		isActive = false;
		pContext->physics->DestroyJoint(mouseJoint);
		mouseJoint = nullptr;
		pContext->physics->DestroyBody(mouseBody);
		mouseBody = nullptr;
		break;
	}
	case InputEvent::EV_MOUSE_MOVED: {
		if (mouseJoint) {
			mouseJoint->SetTarget(g2b(pContext->pViewport->unproject(glm::vec2(ev.x, ev.y))));
			mouseBody->SetTransform(mouseJoint->GetTarget(), 0);
		}
		break;
	}
	default:
		break;
	}
}

void OperationSpring::update(float dt) {
}
