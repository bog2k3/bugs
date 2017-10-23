#pragma once

#include "../renderOpenGL/ViewportCoord.h"

#include <glm/vec3.hpp>

#include <string>

class Label
{
public:

	Label(std::string value, ViewportCoord pos, float z, float textSize, glm::vec3 color, std::string viewportFilter_ = "");

	void setText(std::string text) { value_ = text; }
	void setColor(glm::vec3 rgb) { color_ = rgb; }
	void setTextSize(float size) { textSize_ = size; }
	void setPos(ViewportCoord pos) { pos_ = pos; }

	glm::vec2 boxSize() const;

	void draw();

	bool drawFrame = true;

protected:
	ViewportCoord pos_;
	float z_;
	glm::vec3 color_;
	float textSize_;
	std::string value_;
	std::string viewportFilter_;
};
