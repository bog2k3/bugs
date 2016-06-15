#pragma once

#include <string>
#include <glm/vec3.hpp>

class RenderContext;

class Label
{
public:

	enum class RenderMode {
		WorldSpace,
		ScreenSpace
	};

	Label(std::string const& value, glm::vec3 const& pos, float textSize, glm::vec3 const& color);

	void setText(std::string const& text) { value_ = text; }
	void setColor(glm::vec3 const& rgb) { color_ = rgb; }
	void setTextSize(float size) { textSize_ = size; }
	void setPos(glm::vec3 const& pos) { pos_ = pos; }
	void setRenderMode(RenderMode mode) { renderMode_ = mode; }

	glm::vec2 getBoxSize(RenderContext const& ctx);

	void draw(RenderContext const& ctx);

	bool drawFrame_ = true;

protected:
	glm::vec3 pos_;
	glm::vec3 color_;
	float textSize_;
	std::string value_;
	RenderMode renderMode_ = RenderMode::WorldSpace;
};
