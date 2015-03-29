#pragma once

#include <glm/vec3.hpp>

class RenderContext;

class ScaleDisplay
{
public:
	ScaleDisplay(glm::vec3 pos, int maxPixelsPerUnit);

	void draw(RenderContext const& ctx);

protected:
	glm::vec3 pos_;
	int segmentsXOffset;
	int segmentHeight;
	int labelYOffset;
	int m_MaxSize;
};
