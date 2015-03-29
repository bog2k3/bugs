/*
 * GuiContainerElement.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include "GuiContainerElement.h"
#include "GuiHelper.h"
#include <algorithm>
#include <glm/vec3.hpp>

GuiContainerElement::GuiContainerElement(glm::vec2 position, glm::vec2 size)
	: GuiBasicElement(position, size)
{
}

GuiContainerElement::~GuiContainerElement() {
	children_.clear();
}

void GuiContainerElement::draw(RenderContext const &ctx, glm::vec3 frameTranslation, glm::vec2 frameScale) {
	// draw all children relative to the client area
	frameTranslation += glm::vec3(clientAreaOffset_, getZValue());
	for (auto &e : children_) {
		e->draw(ctx, frameTranslation + glm::vec3(e->getPosition(), e->getZValue()), frameScale);
	}
	// TODO draw frame around focused element:
}

void GuiContainerElement::setSize(glm::vec2 size) {
	glm::vec2 oldSize = getSize();
	GuiBasicElement::setSize(size);
	updateClientArea();
	for (auto e : children_) {
		//TODO: adjust e position and size based on anchors
	}
}

void GuiContainerElement::updateClientArea() {
	clientAreaSize_ = getSize() - clientAreaOffset_ - clientAreaCounterOffset_;
}

void GuiContainerElement::addElement(std::shared_ptr<GuiBasicElement> e) {
	children_.push_back(e);
	e->setCaptureManager(getCaptureManager());
}

void GuiContainerElement::removeElement(std::shared_ptr<GuiBasicElement> e) {
	children_.erase(std::find(children_.begin(), children_.end(), e));
}

void GuiContainerElement::mouseDown(MouseButtons button) {
	GuiBasicElement::mouseDown(button);
	if (elementUnderMouse_)
		elementUnderMouse_->mouseDown(button);
}

void GuiContainerElement::mouseUp(MouseButtons button) {
	GuiBasicElement::mouseUp(button);
	if (elementUnderMouse_)
		elementUnderMouse_->mouseUp(button);
}

void GuiContainerElement::mouseMoved(glm::vec2 delta, glm::vec2 position) {
	GuiBasicElement::mouseMoved(delta, position);
	glm::vec2 clientPos = position - clientAreaOffset_;
	std::shared_ptr<GuiBasicElement> crt = GuiHelper::getTopElementAtPosition(children_, clientPos.x, clientPos.y);
	if (crt != elementUnderMouse_) {
		elementUnderMouse_->mouseLeave();
		elementUnderMouse_ = crt;
		elementUnderMouse_->mouseEnter();
	}
	if (elementUnderMouse_)
		elementUnderMouse_->mouseMoved(delta, clientPos - elementUnderMouse_->getPosition());
}

void GuiContainerElement::clicked(glm::vec2 clickPosition, MouseButtons button) {
	GuiBasicElement::clicked(clickPosition, button);
	if (elementUnderMouse_)
		elementUnderMouse_->clicked(clickPosition - clientAreaOffset_ - elementUnderMouse_->getPosition(), button);
}

void GuiContainerElement::setClientArea(glm::vec2 offset, glm::vec2 counterOffset) {
	clientAreaOffset_ = offset;
	clientAreaCounterOffset_ = counterOffset;
	updateClientArea();
}

void GuiContainerElement::getClientArea(glm::vec2 &outOffset, glm::vec2 &outSize) {
	outOffset = clientAreaOffset_;
	outSize = clientAreaSize_;
}