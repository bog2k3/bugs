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


};

#endif /* GUI_WINDOW_H_ */
