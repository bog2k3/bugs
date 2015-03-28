/*
 * Window.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_WINDOW_H_
#define GUI_WINDOW_H_

#include "GuiContainerElement.h"

class Window: public GuiContainerElement {
public:
	Window(glm::vec2 position, glm::vec2 size);
	virtual ~Window();

	virtual void draw(RenderContext const &ctx, glm::vec3 frameTranslation, glm::vec2 frameScale) override;
	virtual void mouseDown(MouseButtons button) override;
	virtual void mouseUp(MouseButtons button) override;
	virtual void mouseMoved(glm::vec2 delta, glm::vec2 position) override;

private:
	glm::vec2 downPosition_;
};

#endif /* GUI_WINDOW_H_ */
