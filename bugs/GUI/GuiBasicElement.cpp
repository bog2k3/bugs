/*
 * GuiBasicElement.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include "GuiBasicElement.h"

GuiBasicElement::GuiBasicElement(glm::vec2 position, glm::vec2 size)
	: position_(position)
	, size_(size)
{
	updateBBox();
}

GuiBasicElement::~GuiBasicElement() {

}

void GuiBasicElement::setAnchors(Anchors anch) {

}

void GuiBasicElement::setPosition(glm::vec2 position) {

}

void GuiBasicElement::setSize(glm::vec2 size) {

}

void GuiBasicElement::setZValue(float z) {

}


void GuiBasicElement::mouseEnter() {

}

void GuiBasicElement::mouseLeave() {

}

void GuiBasicElement::mouseDown(MouseButtons button) {

}

void GuiBasicElement::mouseUp(MouseButtons button) {

}

void updateBBox() {

}
