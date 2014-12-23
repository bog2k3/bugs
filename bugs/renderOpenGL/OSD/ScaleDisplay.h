#pragma once

#include <glm/vec2.hpp>

class RenderContext;

class ScaleDisplay
{
public:
	ScaleDisplay(int maxPixelSize);

	void render(RenderContext* ctx);

protected:
	// TODO: make multi-instance safe (share video resources)
	glm::vec2 pos_;
	int segmentsXOffset;
	int segmentHeight;
	int labelYOffset;
	int m_MaxSize;
};
