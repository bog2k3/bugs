/*
 * GuiContainerElement.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include "GuiContainerElement.h"
#include <algorithm>

GuiContainerElement::GuiContainerElement(glm::vec2 position, glm::vec2 size)
	: GuiBasicElement(position, size)
{
}

GuiContainerElement::~GuiContainerElement() {
	children_.clear();
}

void GuiContainerElement::setSize(glm::vec2 size) {
	glm::vec2 oldSize = getSize();
	GuiBasicElement::setSize(size);
	for (auto e : children_) {
		//TODO: adjust e position and size based on anchors
	}
}

void GuiContainerElement::addElement(std::shared_ptr<GuiBasicElement> e) {
	children_.push_back(e);
}

void GuiContainerElement::removeElement(std::shared_ptr<GuiBasicElement> e) {
	children_.erase(std::find(children_.begin(), children_.end(), e));
}

void GuiContainerElement::mouseEnter() {
	GuiBasicElement::mouseEnter();
}

void GuiContainerElement::mouseLeave() {
	GuiBasicElement::mouseLeave();
}

void GuiContainerElement::mouseDown(MouseButtons button) {
	GuiBasicElement::mouseDown(button);
}

void GuiContainerElement::mouseUp(MouseButtons button) {
	GuiBasicElement::mouseUp(button);
}

void GuiContainerElement::mouseMoved(glm::vec2 delta, glm::vec2 position) {
	GuiBasicElement::mouseMoved(delta, position);
}

void GuiContainerElement::clicked(glm::vec2 clickPosition, MouseButtons button) {
	GuiBasicElement::clicked(clickPosition, button);
}
