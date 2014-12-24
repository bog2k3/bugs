#pragma once

#include "../../drawable.h"
#include <glm/vec2.hpp>

class RenderContext;

class ScaleDisplay
{
public:
	ScaleDisplay(glm::vec2 pos, int maxPixelsPerUnit);
	bool operator==(ScaleDisplay const& other) const { return false; }

protected:
	glm::vec2 pos_;
	int segmentsXOffset;
	int segmentHeight;
	int labelYOffset;
	int m_MaxSize;

	friend void draw<ScaleDisplay>(ScaleDisplay& s, RenderContext& ctx);
	friend void draw<ScaleDisplay*>(ScaleDisplay*& s, RenderContext& ctx);
};

template<>
void draw(ScaleDisplay& s, RenderContext& ctx);

template<>
void draw(ScaleDisplay*& s, RenderContext& ctx);
