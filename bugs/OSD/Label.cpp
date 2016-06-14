#include "Label.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Viewport.h"
#include "../renderOpenGL/GLText.h"
#include "../renderOpenGL/Shape2D.h"
#include "../math/math2D.h"

Label::Label(std::string const& value, glm::vec3 const& pos, float textSize, glm::vec3 const& color)
	: pos_(pos)
	, color_(color)
	, textSize_(textSize)
	, value_(value) {
}

glm::vec2 Label::getBoxSize(RenderContext const& ctx) {
	return ctx.text->getTextRect(value_, textSize_);
}

void Label::draw(RenderContext const& ctx) {
	glm::vec3 renderPos = pos_;
	if (renderMode_ == RenderMode::WorldSpace)
		vec3xy(renderPos) = ctx.viewport->project(vec3xy(renderPos));
	if (ctx.viewport->containsPoint(vec3xy(renderPos)))
		ctx.text->print(value_, renderPos.x, renderPos.y, renderPos.z, textSize_, color_);
	if (drawFrame_) {
		glm::vec2 rectSize = ctx.text->getTextRect(value_, textSize_);
		ctx.shape->drawRectangle(vec3xy(renderPos) - glm::vec2(5, 5), renderPos.z, rectSize + glm::vec2(5, 5), color_);
	}
}
