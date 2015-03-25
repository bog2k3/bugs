/*
 * GuiSystem.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#include "GuiSystem.h"
#include "IGuiElement.h"
#include "../input/InputEvent.h"
#include "../utils/log.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <algorithm>

void GuiSystem::addElement(std::shared_ptr<IGuiElement> &e) {
	elements_.push_back(e);
	e->setCaptureManager(this);
}

void GuiSystem::removeElement(std::shared_ptr<IGuiElement> &e) {
	elements_.erase(std::find(elements_.begin(), elements_.end(), e));
	e->setCaptureManager(nullptr);
}

void GuiSystem::draw(RenderContext const &ctx) {
	for (auto &e : elements_)
		e->draw(ctx, glm::vec3(0), glm::vec2(1));
}

void GuiSystem::handleInput(InputEvent &ev) {
	if (ev.isConsumed())
		return;
	switch (ev.type) {
	case InputEvent::EV_KEY_DOWN:
		if (pFocusedElement_)
			pFocusedElement_->keyDown(ev.key);
		break;
	case InputEvent::EV_KEY_UP:
		if (pFocusedElement_)
			pFocusedElement_->keyUp(ev.key);
		break;
	case InputEvent::EV_KEY_CHAR:
		if (pFocusedElement_)
			pFocusedElement_->keyChar(ev.ch);
		break;
	case InputEvent::EV_MOUSE_DOWN:
		if (pCaptured)
			pCaptured->mouseDown((MouseButtons)ev.mouseButton);
		else {
			if (lastUnderMouse) {
				lastUnderMouse->mouseDown((MouseButtons)ev.mouseButton);
				if (pFocusedElement_ != lastUnderMouse) {
					pFocusedElement_->focusLost();
					pFocusedElement_ = lastUnderMouse;
					pFocusedElement_->focusGot();
				}
			}
		}
		break;
	case InputEvent::EV_MOUSE_UP:
		if (pCaptured)
			pCaptured->mouseUp((MouseButtons)ev.mouseButton);
		else {
			if (lastUnderMouse)
				lastUnderMouse->mouseUp((MouseButtons)ev.mouseButton);
		}
		break;
	case InputEvent::EV_MOUSE_MOVED:
		if (pCaptured)
			pCaptured->mouseMoved(glm::vec2(ev.dx, ev.dy), glm::vec2(ev.x, ev.y));
		else {
			IGuiElement *crt = getElementUnderMouse(ev.x, ev.y);
			if (crt != lastUnderMouse) {
				if (lastUnderMouse)
					lastUnderMouse->mouseLeave();
				if (crt) {
					crt->mouseEnter();
					crt->mouseMoved(glm::vec2(ev.dx, ev.dy), glm::vec2(ev.x, ev.y));
				}
				lastUnderMouse = crt;
			}
		}
		break;
	case InputEvent::EV_MOUSE_SCROLL:
		if (pCaptured)
			pCaptured->mouseScroll(ev.dz);
		else {
			if (lastUnderMouse)
				lastUnderMouse->mouseScroll(ev.dz);
		}
		break;
	default:
		LOGLN("unknown event type: " << ev.type);
	}
}

IGuiElement* GuiSystem::getElementUnderMouse(float x, float y) {
	std::vector<IGuiElement*> vec;
	for (auto &e : elements_) {
		glm::vec2 min, max;
		e->getBoundingBox(min, max);
		if (x >= min.x && y >= min.y && x <= max.x && y <= max.y)
			vec.push_back(e.get());
	}
	IGuiElement* top = nullptr;
	float topZ = 0.f;
	for (auto &e : vec)
		if (e->getZValue() >= topZ) {
			topZ = e->getZValue();
			top = e;
		}
	return top;
}
