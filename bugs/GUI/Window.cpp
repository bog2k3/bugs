/*
 * Window.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include "Window.h"
#include "GuiTheme.h"
#include "ICaptureManager.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include "../math/math2D.h"

Window::Window(glm::vec2 position, glm::vec2 size)
	: GuiContainerElement(position, size)
{
	setClientArea(glm::vec2(3, 20), glm::vec2(3, 3));
}

Window::~Window() {
	// TODO Auto-generated destructor stub
}

void Window::draw(RenderContext const &ctx, glm::vec3 frameTranslation, glm::vec2 frameScale) {
	glm::ivec3 trans(frameTranslation);
	// draw frame
	glm::vec2 scaledSize = getSize();
	scaledSize.x *= frameScale.x;
	scaledSize.y *= frameScale.y;
	ctx.shape->drawRectangleFilled(vec3xy(trans), frameTranslation.z, scaledSize, GuiTheme::getWindowColor());
	ctx.shape->drawRectangle(vec3xy(trans), frameTranslation.z, scaledSize, GuiTheme::getWindowFrameColor());

	// draw client area frame:
	glm::vec2 clientOffset, clientSize;
	getClientArea(clientOffset, clientSize);
	clientSize.x *= frameScale.x;
	clientSize.y *= frameScale.y;
	ctx.shape->drawRectangleFilled(vec3xy(trans)+clientOffset, frameTranslation.z+0.1f,
			clientSize, GuiTheme::getClientColor());
	ctx.shape->drawRectangle(vec3xy(trans)+clientOffset, frameTranslation.z+0.1f,
			clientSize, GuiTheme::getClientFrameColor());

	// now draw contents:
	GuiContainerElement::draw(ctx, glm::vec3(trans)+glm::vec3(0,0,0.5f), frameScale);
}

void Window::mouseDown(MouseButtons button) {
	if (button == MouseButtons::Left) {
		downPosition_ = getLastMousePosition();
		if (getPointedElement() == nullptr)
			getCaptureManager()->setMouseCapture(this);
	}
	GuiContainerElement::mouseDown(button);
}

void Window::mouseUp(MouseButtons button) {
	if (button == MouseButtons::Left && getPointedElement() == nullptr)
		getCaptureManager()->setMouseCapture(nullptr);
	GuiContainerElement::mouseUp(button);
}

void Window::mouseMoved(glm::vec2 delta, glm::vec2 position) {
	if (isMousePressed(MouseButtons::Left) && getPointedElement() == nullptr)
		setPosition(getPosition() + delta);
	else
		GuiContainerElement::mouseMoved(delta, position);
}
