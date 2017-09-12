#include "Label.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Viewport.h"
#include "../renderOpenGL/ViewportCoord.h"
#include "../renderOpenGL/GLText.h"
#include "../renderOpenGL/Shape2D.h"
#include "../math/math3D.h"

Label::Label(std::string value, ViewportCoord pos, float z, float textSize, glm::vec3 color, std::set<std::string> viewportFilters)
	: pos_(pos)
	, z_(z)
	, color_(color)
	, textSize_(textSize)
	, value_(value)
	, viewportFilters_(viewportFilters) {
}

glm::vec2 Label::boxSize() const {
	return GLText::get()->getTextRect(value_, textSize_);
}

void Label::draw() {
	GLText::get()->print(value_, pos_, z_, textSize_, color_, viewportFilters_);
	if (drawFrame) {
		glm::vec2 rectSize = boxSize();
		auto vsz = ViewportCoord(rectSize.x, rectSize.y);
		Shape2D::get()->drawRectangle(
				pos_ + ViewportCoord{5, 5},
				z_,
				vsz + ViewportCoord{5, 5},
				color_);
	}
}
