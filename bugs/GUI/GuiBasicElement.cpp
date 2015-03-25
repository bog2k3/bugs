/*
 * GuiBasicElement.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include "GuiBasicElement.h"
#include <glm/geometric.hpp>

GuiBasicElement::GuiBasicElement(glm::vec2 position, glm::vec2 size)
	: position_(position)
	, size_(size)
{
	updateBBox();
}

GuiBasicElement::~GuiBasicElement() {

}

void GuiBasicElement::setPosition(glm::vec2 position) {
	position_ = position;
	updateBBox();
}

void GuiBasicElement::setSize(glm::vec2 size) {
	size_ = size;
	updateBBox();
}

void GuiBasicElement::mouseEnter() {
	isMouseIn_ = true;
}

void GuiBasicElement::mouseLeave() {
	isMouseIn_ = false;
}

void GuiBasicElement::mouseDown(MouseButtons button) {
	if (isMouseIn_) {
		isMousePressed_[(int)button] = true;
		mouseTravel_[(int)button] = glm::vec2(0);
	}
}

void GuiBasicElement::mouseUp(MouseButtons button) {
	isMousePressed_[(int)button] = false;
	if (isMouseIn_ && glm::length(mouseTravel_[(int)button]) <= MAX_CLICK_TRAVEL)
		clicked(lastMousePosition_, button);
}

void GuiBasicElement::mouseMoved(glm::vec2 delta, glm::vec2 position) {
	for (int i=0; i<3; i++)
		mouseTravel_[i] += delta;
	lastMousePosition_ = position;
}

void GuiBasicElement::updateBBox() {

}
