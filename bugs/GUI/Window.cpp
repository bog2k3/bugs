/*
 * Window.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include "Window.h"
#include "GuiTheme.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include "../math/math2D.h"

Window::Window(glm::vec2 position, glm::vec2 size)
	: GuiContainerElement(position, size)
{
	setClientArea(glm::vec2(2, 20), glm::vec2(2, 2));
}

Window::~Window() {
	// TODO Auto-generated destructor stub
}

void Window::draw(RenderContext const &ctx, glm::vec3 frameTranslation, glm::vec2 frameScale) {
	// draw frame
	glm::vec2 scaledSize = getSize();
	scaledSize.x *= frameScale.x;
	scaledSize.y *= frameScale.y;
	ctx.shape->drawRectangle(vec3xy(frameTranslation), frameTranslation.z, scaledSize, 0, GuiTheme::getWindowFrameColor());

	// draw client area frame:
	glm::vec2 clientOffset, clientSize;
	getClientArea(clientOffset, clientSize);
	clientSize.x *= frameScale.x;
	clientSize.y *= frameScale.y;
	ctx.shape->drawRectangle(vec3xy(frameTranslation)+clientOffset, frameTranslation.z+getZValue(),
			clientSize, 0, GuiTheme::getClientFrameColor());

	// now draw contents:
	GuiContainerElement::draw(ctx, frameTranslation, frameScale);
}
