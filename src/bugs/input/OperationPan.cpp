/*
 * OperationPan.cpp
 *
 *  Created on: Nov 4, 2014
 *      Author: bog
 */

#include "OperationPan.h"
#include "../../renderOpenGL/IRenderer.h"

OperationPan::OperationPan(IRenderer* renderer)
	: pRenderer(renderer)
	, isFlyActive(false)
	, isDragging(false)
	, lastDelta{}
	, lastIndex(0)
	, frictionFactor(1.1)
	, flySpeed()
{
}

OperationPan::~OperationPan() {
	// TODO Auto-generated destructor stub
}

void OperationPan::handleInput(InputEvent& ev) {
	switch (ev.type) {
	case InputEvent::EV_MOUSE_DOWN: {
		if (ev.mouseButton != InputEvent::MB_LEFT)
			break;
		isDragging = true;
		isFlyActive = false;
		lastIndex = 0;
		lastDelta[lastIndex] = glm::vec2(ev.dx, ev.dy);
		break;
	}
	case InputEvent::EV_MOUSE_MOVED: {
		lastIndex = (lastIndex + 1) % nFilter;
		lastDelta[lastIndex] = glm::vec2(ev.dx, ev.dy);
		glm::vec2 cam = pRenderer->getCameraPos();
		cam += glm::vec2(ev.dx, ev.dy) / pRenderer->getZoomLevel();
		pRenderer->moveCameraTo(cam);
		break;
	}
	default:
		break;
	}
}

void OperationPan::update(float dt) {
	if (isFlyActive) {
		glm::vec2 cam = pRenderer->getCameraPos();
		cam += flySpeed * dt;
		flySpeed /= frictionFactor;
		if (flySpeed.length() * pRenderer->getZoomLevel() < 5) // less than 5 screen pixels per second, then stop
			isFlyActive = false;
	}
}
