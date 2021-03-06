/*
 * GuiSystem.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#include "GuiSystem.h"
#include "IGuiElement.h"
#include "GuiHelper.h"
#include "../input/InputEvent.h"
#include "../utils/log.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include "../math/math3D.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <algorithm>

void GuiSystem::addElement(std::shared_ptr<IGuiElement> e) {
	elements_.push_back(e);
	e->setCaptureManager(this);
	if (elements_.size() > 1)
		e->setZValue(elements_[0]->getZValue()+1);
	normalizeZValuesAndSort(nullptr);
}

void GuiSystem::removeElement(std::shared_ptr<IGuiElement> e) {
	elements_.erase(std::find(elements_.begin(), elements_.end(), e));
	e->setCaptureManager(nullptr);
}

void GuiSystem::draw(RenderContext const &ctx) {
	for (auto &e : elements_)
	{
		glm::vec2 bboxMin, bboxMax;
		e->getBoundingBox(bboxMin, bboxMax);
		e->draw(ctx, glm::vec3(bboxMin, e->getZValue()), glm::vec2(1));
	}
}

void GuiSystem::normalizeZValuesAndSort(IGuiElement* top) {
	float maxZ = 0;
	if (top)
		maxZ = top->getZValue();
	else
		for (auto &e : elements_)
			if (e->getZValue() > maxZ)
				maxZ = e->getZValue();

	float invMax = maxZ ? 1.f / maxZ : 1;
	for (unsigned i=0; i<elements_.size(); i++) {
		for (unsigned j=i+1; j<elements_.size(); j++)
			if (elements_[j]->getZValue() < elements_[i]->getZValue())
				xchg(elements_[i], elements_[j]);
		elements_[i]->setZValue(elements_[i]->getZValue() * invMax);
	}
}

void GuiSystem::handleInput(InputEvent &ev) {
	if (ev.isConsumed())
		return;
	switch (ev.type) {
	case InputEvent::EV_KEY_DOWN:
		if (pFocusedElement_) {
			pFocusedElement_->keyDown(ev.key);
			ev.consume();
		}
		break;
	case InputEvent::EV_KEY_UP:
		if (pFocusedElement_) {
			pFocusedElement_->keyUp(ev.key);
			ev.consume();
		}
		break;
	case InputEvent::EV_KEY_CHAR:
		if (pFocusedElement_) {
			pFocusedElement_->keyChar(ev.ch);
			ev.consume();
		}
		break;
	case InputEvent::EV_MOUSE_DOWN:
		if (pCaptured) {
			pCaptured->mouseDown((MouseButtons)ev.mouseButton);
			ev.consume();
		} else {
			if (lastUnderMouse) {
				lastUnderMouse->mouseDown((MouseButtons)ev.mouseButton);
				int lastZ = 1;
				if (pFocusedElement_ != lastUnderMouse) {
					if (pFocusedElement_) {
						pFocusedElement_->focusLost();
						lastZ += pFocusedElement_->getZValue();
					}
					pFocusedElement_ = lastUnderMouse;
					pFocusedElement_->focusGot();
					pFocusedElement_->setZValue(lastZ+1);
					normalizeZValuesAndSort(pFocusedElement_);
				}
				ev.consume();
			}
		}
		break;
	case InputEvent::EV_MOUSE_UP:
		if (pCaptured) {
			pCaptured->mouseUp((MouseButtons)ev.mouseButton);
			ev.consume();
		} else {
			if (lastUnderMouse) {
				lastUnderMouse->mouseUp((MouseButtons)ev.mouseButton);
				ev.consume();
			}
		}
		break;
	case InputEvent::EV_MOUSE_MOVED:
		if (pCaptured) {
			pCaptured->mouseMoved(glm::vec2(ev.dx, ev.dy), glm::vec2(ev.x, ev.y));
			ev.consume();
		} else {
			IGuiElement *crt = getElementUnderMouse(ev.x, ev.y);
			if (crt != lastUnderMouse) {
				if (lastUnderMouse)
					lastUnderMouse->mouseLeave();
				lastUnderMouse = crt;
				if (lastUnderMouse) {
					lastUnderMouse->mouseEnter();
				}
			}
			if (lastUnderMouse)
				lastUnderMouse->mouseMoved(glm::vec2(ev.dx, ev.dy), glm::vec2(ev.x, ev.y));
		}
		break;
	case InputEvent::EV_MOUSE_SCROLL:
		if (pCaptured) {
			pCaptured->mouseScroll(ev.dz);
			ev.consume();
		} else {
			if (lastUnderMouse) {
				lastUnderMouse->mouseScroll(ev.dz);
				ev.consume();
			}
		}
		break;
	default:
		LOGLN("unknown event type: " << ev.type);
	}
}

IGuiElement* GuiSystem::getElementUnderMouse(float x, float y) {
	return GuiHelper::getTopElementAtPosition(elements_, x, y).get();
}
