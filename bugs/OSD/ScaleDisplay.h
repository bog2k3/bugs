#pragma once

#include <glm/vec2.hpp>

class RenderContext;

class ScaleDisplay
{
public:
	ScaleDisplay(glm::vec2 pos, int maxPixelsPerUnit);

	void draw(RenderContext const& ctx);

protected:
	glm::vec2 pos_;
	int segmentsXOffset;
	int segmentHeight;
	int labelYOffset;
	int m_MaxSize;
};
