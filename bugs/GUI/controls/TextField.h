/*
 * TextField.h
 *
 *  Created on: Mar 29, 2015
 *      Author: alexandra
 */

#ifndef GUI_CONTROLS_TEXTFIELD_H_
#define GUI_CONTROLS_TEXTFIELD_H_

#include "../GuiBasicElement.h"
#include "../../utils/Event.h"
#include <string>

class TextField: public GuiBasicElement {
public:
	TextField(glm::vec2 pos, glm::vec2 size, std::string initialText);
	virtual ~TextField();

	std::string getText();

	virtual void keyDown(int keyCode) override;
	virtual void keyChar(char c) override;
	virtual void draw(RenderContext const &ctx, glm::vec3 frameTranslation, glm::vec2 frameScale) override;

	Event<void(TextField*)> onTrigger;

protected:
	static constexpr int maxTextbufferSize = 512;
	char textBuffer_[maxTextbufferSize];
	int bufPos_=0;
	int bufSize_=0;
};

#endif /* GUI_CONTROLS_TEXTFIELD_H_ */
