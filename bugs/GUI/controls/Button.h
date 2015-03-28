/*
 * Button.h
 *
 *  Created on: Mar 28, 2015
 *      Author: bog
 */

#ifndef GUI_CONTROLS_BUTTON_H_
#define GUI_CONTROLS_BUTTON_H_

#include "../GuiBasicElement.h"
#include "../../utils/Event.h"
#include <string>

class Button: public GuiBasicElement {
public:
	Button(glm::vec2 pos, glm::vec2 size, std::string text);
	virtual ~Button() override;

	Event<void(Button*)> onClick;

	void setText(std::string text) { text_ = text; }

protected:
	virtual void clicked(glm::vec2 clickPosition, MouseButtons button) override;
	virtual void draw(RenderContext const& ctx, glm::vec3 frameTranslation, glm::vec2 frameScale) override;

private:
	std::string text_;
};

#endif /* GUI_CONTROLS_BUTTON_H_ */
