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
		windowFrameColor = glm::vec3(0.f, 0.f, 0.7f);
		clientFrameColor = glm::vec3(0.1f, 0.1f, 0.5f);
	}
};

#endif /* GUI_DEFAULTTHEME_H_ */
