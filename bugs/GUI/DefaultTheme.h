/*
 * DefaultTheme.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_DEFAULTTHEME_H_
#define GUI_DEFAULTTHEME_H_

#include "GuiTheme.h"

class DefaultTheme : public GuiTheme {
public:
	DefaultTheme() {
		windowColor = glm::vec4(0.2f, 0.2f, 0.6f, 0.8f);
		windowFrameColor = glm::vec4(0.f, 0.f, 0.95f, 1.f);
		clientColor = glm::vec4(0.9f, 0.9f, 0.95f, 0.05f);
		clientFrameColor = glm::vec4(0.4f, 0.4f, 0.95f, 0.6f);
		buttonColor = glm::vec4(0.2, 0.4, 0.7, 1.f);
		buttonFrameColor = glm::vec4(0.1, 0.2, 0.3, 0.7f);
		//buttonTextColor = glm::vec4(0.6, 0.6, 1.0, 1.f);
		buttonTextColor = glm::vec4(0, 0, 0, 1.f);
		textFieldColor = glm::vec4(0.8f, 0.8f, 1.f, 0.8f);
	}
};

#endif /* GUI_DEFAULTTHEME_H_ */
