
#include "ScaleDisplay.h"
#include "../../renderOpenGL/RenderContext.h"
#include "../../renderOpenGL/Viewport.h"
#include "../../renderOpenGL/Camera.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/GLText.h"
#include <math.h>
#include <glm/vec3.hpp>
#include <stdio.h>

static const glm::vec3 LINE_COLOR(0.8f, 0.8f, 0.8f);
static const glm::vec3 TEXT_COLOR(1.f, 1.f, 1.f);

ScaleDisplay::ScaleDisplay(glm::vec2 pos, int maxPixelsPerUnit)
	: pos_(pos)
	, segmentsXOffset(50)
	, segmentHeight(10)
	, labelYOffset(-12)
	, m_MaxSize(maxPixelsPerUnit)
{
}

template<>
void draw(ScaleDisplay& s, RenderContext& ctx) {
	float pixelsPerUnit = ctx.viewport->getCamera()->getZoomLevel();
	int exponent = 0;

	if (pixelsPerUnit > s.m_MaxSize) {
		// small scale
		while (pixelsPerUnit > s.m_MaxSize) {
			exponent--;
			pixelsPerUnit /= 10;
		}
	} else if (pixelsPerUnit < s.m_MaxSize) {
		// large scale
		while (pixelsPerUnit*10 <= s.m_MaxSize) {
			exponent++;
			pixelsPerUnit *= 10;
		}
	}

	float segIncrement = 1.0f;
	int segments = (int) floor(s.m_MaxSize / pixelsPerUnit);
	if (segments == 1) {
		segments = 5;
		segIncrement = 0.2f;
	/*} else if (segments == 2) {
		segments = 8;
		segIncrement = 0.25f;*/
	} else if (segments <= 3) {
		segments *= 2;
		segIncrement = 0.5f;
	}
	int nVertex = 1 + segments * 3;
	float cx = (float)s.pos_.x + s.segmentsXOffset;
	float cy = (float)s.pos_.y - 1;
	glm::vec2 vList[31]; // 31 is max vertex for max_seg=10
	for (int i=0; i<segments; i++) {
		int localSegHeight = (int)(i*segIncrement) == (i*segIncrement) ? s.segmentHeight : s.segmentHeight / 2;
		vList[i*3+0] = glm::vec2(cx, cy-localSegHeight);
		vList[i*3+1] = glm::vec2(cx, cy);
		cx += (float)pixelsPerUnit * segIncrement;
		vList[i*3+2] = glm::vec2(cx, cy);
	}
	vList[nVertex-1] = glm::vec2(cx, cy-s.segmentHeight);

	ctx.shape->drawLineStripViewport(vList, nVertex, 0, LINE_COLOR, *ctx.viewport);

	char scaleLabel[100];

	snprintf(scaleLabel, 100, "(10^%d)", exponent);
	ctx.text->print(scaleLabel, s.pos_.x, s.pos_.y, 14, TEXT_COLOR);
	for (int i=0; i<segments+1; i++) {
		snprintf(scaleLabel, 100, "%g", i*segIncrement);
		int localSegHeight = (int)(i*segIncrement) == (i*segIncrement) ? 0 : s.segmentHeight / 2;
		ctx.text->print(scaleLabel, s.pos_.x - localSegHeight +s.segmentsXOffset+i*(int)(pixelsPerUnit*segIncrement), s.pos_.y - 10 + localSegHeight, 12, TEXT_COLOR);
	}
}

template<>
void draw(ScaleDisplay*& s, RenderContext& ctx) {
	draw(*s, ctx);
}
