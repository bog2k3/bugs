/*
 * OperationPan.cpp
 *
 *  Created on: Nov 4, 2014
 *      Author: bog
 */

#include "OperationPan.h"
#include "InputEvent.h"
#include "../renderOpenGL/IRenderer.h"
#include <GLFW/glfw3.h>

OperationPan::OperationPan(IRenderer* renderer)
	: pRenderer(renderer)
	, isFlyActive(false)
	, isDragging(false)
	, lastDelta{}
	, filterTimes{}
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
		filterTimes[lastIndex] = glfwGetTime();
		break;
	}
	case InputEvent::EV_MOUSE_UP: {
		if (ev.mouseButton != InputEvent::MB_LEFT)
			break;
		isDragging = false;
		isFlyActive = true;
		// compute average drag over the last nFilter frames
		flySpeed = glm::vec2(0);
		float minTime = 1e+20;
		for (unsigned i=0; i<nFilter; i++) {
			flySpeed += lastDelta[i];
			if (filterTimes[i] < minTime)
				minTime = filterTimes[i];
		}
		flySpeed /= (glfwGetTime()- minTime) * pRenderer->getZoomLevel() * 2;
		flySpeed.x *= -1;
		break;
	}
	case InputEvent::EV_MOUSE_MOVED: {
		if (!isDragging)
			break;
		lastIndex = (lastIndex + 1) % nFilter;
		lastDelta[lastIndex] = glm::vec2(ev.dx, ev.dy);
		filterTimes[lastIndex] = glfwGetTime();
		pRenderer->moveCamera(glm::vec2(-ev.dx, ev.dy) / pRenderer->getZoomLevel());
		break;
	}
	case InputEvent::EV_MOUSE_SCROLL: {
		float factor = ev.dz < 0 ? 0.90f : 1.10f;
		pRenderer->setZoomLevel(pRenderer->getZoomLevel() * factor);
		break;
	}
	default:
		break;
	}
}

void OperationPan::update(float dt) {
	if (isFlyActive) {
		pRenderer->moveCamera(flySpeed * dt);
		flySpeed /= frictionFactor;
		if (flySpeed.length() * pRenderer->getZoomLevel() < 5) // less than 5 screen pixels per second, then stop
			isFlyActive = false;
	}
}
