/*
 * GuiTheme.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUITHEME_H_
#define GUI_GUITHEME_H_

#include <glm/vec4.hpp>
#include <memory>

class GuiTheme {
public:
	static void setActiveTheme(std::shared_ptr<GuiTheme> theme) { activeTheme_ = theme; }

	static glm::vec4 getWindowColor() { return activeTheme_->windowColor; }
	static glm::vec4 getWindowFrameColor() { return activeTheme_->windowFrameColor; }
	static glm::vec4 getClientColor() { return activeTheme_->clientColor; }
	static glm::vec4 getClientFrameColor() { return activeTheme_->clientFrameColor; }

protected:
	glm::vec4 windowColor;
	glm::vec4 windowFrameColor;
	glm::vec4 clientColor;
	glm::vec4 clientFrameColor;

private:
	static std::shared_ptr<GuiTheme> activeTheme_;
};

#endif /* GUI_GUITHEME_H_ */
