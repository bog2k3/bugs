/*
 * Button.cpp
 *
 *  Created on: Mar 28, 2015
 *      Author: bog
 */

#include "Button.h"
#include "../GuiTheme.h"
#include "../../renderOpenGL/RenderContext.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/GLText.h"
#include "../../math/math3D.h"

Button::Button(glm::vec2 pos, glm::vec2 size, std::string text)
	: GuiBasicElement(pos, size)
	, text_(text) {
}

Button::~Button() {
}

void Button::clicked(glm::vec2 clickPosition, MouseButtons button) {
	if (button == MouseButtons::Left)
		onClick.trigger(this);
}

void Button::draw(RenderContext const& ctx, glm::vec3 frameTranslation, glm::vec2 frameScale) {
	Shape2D::get()->drawRectangleFilled(
			vec3xy(frameTranslation) + glm::vec2(2,2),	// TODO switch all GUI controls from absolute coordinates to ViewportCoords
			frameTranslation.z,
			(getSize()-glm::vec2(4,4)) * frameScale,
			GuiTheme::getButtonColor());
	Shape2D::get()->drawRectangle(
			vec3xy(frameTranslation),
			frameTranslation.z,
			getSize() * frameScale,
			GuiTheme::getButtonFrameColor());
	int tx = frameTranslation.x + 10;
	int ty = frameTranslation.y + 15;
	int tz = frameTranslation.z + 1;
	GLText::get()->print(text_, {tx, ty}, tz, 14, GuiTheme::getButtonTextColor());
}
