/*
 * GuiTheme.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUITHEME_H_
#define GUI_GUITHEME_H_

#include <glm/vec3.hpp>
#include <memory>

class GuiTheme {
public:
	static void setActiveTheme(std::shared_ptr<GuiTheme> theme) { activeTheme_ = theme; }

	static glm::vec3 getWindowFrameColor() { return activeTheme_->windowFrameColor; }
	static glm::vec3 getClientFrameColor() { return activeTheme_->clientFrameColor; }

protected:
	glm::vec3 windowFrameColor;
	glm::vec3 clientFrameColor;

private:
	static std::shared_ptr<GuiTheme> activeTheme_;
};

#endif /* GUI_GUITHEME_H_ */
