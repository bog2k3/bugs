
#include "ScaleDisplay.h"
#include "../RenderContext.h"
#include "../Viewport.h"
#include "../Camera.h"
#include "../Shape2D.h"
#include "../GLText.h"
#include <math.h>
#include <glm/vec3.hpp>
#include <stdio.h>

static const glm::vec3 LINE_COLOR(0.8f, 0.8f, 0.8f);

ScaleDisplay::ScaleDisplay(int maxPixelSize)
	: pos_(0)
	, segmentsXOffset(50)
	, segmentHeight(10)
	, labelYOffset(-12)
	, m_MaxSize(maxPixelSize)
{
}

void ScaleDisplay::render(RenderContext* ctx)
{
	float pixelsPerUnit = ctx->viewport->getCamera()->getZoomLevel();
	int exponent = 0;

	if (pixelsPerUnit > m_MaxSize) {
		// small scale
		while (pixelsPerUnit > m_MaxSize) {
			exponent--;
			pixelsPerUnit /= 10;
		}
	} else if (pixelsPerUnit < m_MaxSize) {
		// large scale
		while (pixelsPerUnit*10 <= m_MaxSize) {
			exponent++;
			pixelsPerUnit *= 10;
		}
	}

	float segIncrement = 1.0f;
	int segments = (int) floor(m_MaxSize / pixelsPerUnit);
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
	float cx = (float)pos_.x + segmentsXOffset;
	float cy = (float)pos_.y - 1;
	glm::vec2 vList[31]; // 31 is max vertex for max_seg=10
	for (int i=0; i<segments; i++) {
		int localSegHeight = (int)(i*segIncrement) == (i*segIncrement) ? segmentHeight : segmentHeight / 2;
		vList[i*3+0] = glm::vec2(cx, cy-localSegHeight);
		vList[i*3+1] = glm::vec2(cx, cy);
		cx += (float)pixelsPerUnit * segIncrement;
		vList[i*3+2] = glm::vec2(cx, cy);
	}
	vList[nVertex-1] = glm::vec2(cx, cy-segmentHeight);

	ctx->shape->drawLineList(vList, nVertex, 0, LINE_COLOR);

	char scaleLabel[100];

	snprintf(scaleLabel, 100, "(10^%d)", exponent);
	ctx->text->print(scaleLabel, pos_.x, pos_.y, 12);
	for (int i=0; i<segments+1; i++) {
		snprintf(scaleLabel, 100, "%g", i*segIncrement);
		int localSegHeight = (int)(i*segIncrement) == (i*segIncrement) ? 0 : segmentHeight / 2;
		ctx->text->print(scaleLabel, pos_.x+segmentsXOffset+i*(int)(pixelsPerUnit*segIncrement), pos_.y + localSegHeight, 12);
	}
}
