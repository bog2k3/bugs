/*
 * SignalViewer.cpp
 *
 *  Created on: Jan 24, 2016
 *      Author: bog
 */

#include "SignalViewer.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include "../renderOpenGL/GLText.h"
#include "../renderOpenGL/Viewport.h"
#include "../math/math2D.h"

SignalDataSource::SignalDataSource(float* pValue, int maxSamples, float sampleInterval)
	: pValue_(pValue), capacity_(maxSamples), sampleInterval_(sampleInterval) {
	samples_ = new float[maxSamples];
}

SignalDataSource::~SignalDataSource() {
	delete [] samples_;
}

void SignalDataSource::update(float dt) {
	timeSinceLastSample_ += dt;
	if (timeSinceLastSample_ < sampleInterval_)
		return;

	timeSinceLastSample_ -= sampleInterval_;

	if (n_ < capacity_)
		samples_[n_++] = *pValue_;
	else {
		samples_[zero_] = *pValue_;
		zero_ = (zero_+1) % n_;
	}
}

SignalViewer::SignalViewer(glm::vec3 const& uniformPos, glm::vec2 const& uniformSize)
	: uPos_(uniformPos), uSize_(uniformSize) {
}

SignalViewer::~SignalViewer() {
}

void SignalViewer::addSignal(std::string const& name, float* pValue, int maxSamples, float sampleInterval, glm::vec3 const& rgb) {
	sourceInfo_.push_back(DataInfo(std::unique_ptr<SignalDataSource>(new SignalDataSource(pValue, maxSamples, sampleInterval)), name, rgb));
}

void SignalViewer::update(float dt) {
	for (auto &s : sourceInfo_)
		s.source_->update(dt);
}

void SignalViewer::draw(RenderContext const& ctx) {
	constexpr float yDivisionSize = 30; // pixels
	const glm::vec3 frameColor(0.3f, 0.3f, 0.3f);

	glm::vec2 pos = vec3xy(uPos_);
	pos.x *= ctx.viewport->getWidth();
	pos.y *= ctx.viewport->getHeight();
	glm::vec2 size = uSize_;
	size.x *= ctx.viewport->getWidth();
	size.y *= ctx.viewport->getHeight();
	for (auto &s : sourceInfo_) {
		ctx.shape->setViewportSpaceDraw(true);
		ctx.shape->drawRectangle(pos, uPos_.z, size, frameColor);
		ctx.text->print(s.name_, pos.x, pos.y, uPos_.z, 14, s.color_);
		float sMin = 1.e20f, sMax = -1.e20f;
		// scan all samples and seek min/max values:
		for (uint i=0; i<s.source_->getNumSamples(); i++) {
			float si = s.source_->getSample(i);
			if (si < sMin)
				sMin = si;
			if (si > sMax)
				sMax = si;
		}
		// smooth out zoom level:
		if (sMin > s.lastMinValue_)
			sMin = sMin*0.1f + s.lastMinValue_*0.9f;
		if (sMax < s.lastMaxValue_)
			sMax = sMax*0.1f + s.lastMaxValue_*0.9f;
		s.lastMinValue_ = sMin;
		s.lastMaxValue_ = sMax;
		// draw samples:
		float xAxisZoom = size.x / s.source_->getCapacity();
		float yScale = size.y / (sMax - sMin);
		glm::vec2 prev(pos.x, pos.y + size.y * 0.5f);
		for (uint i=0; i<s.source_->getNumSamples(); i++) {
			glm::vec2 crt(prev.x + xAxisZoom, pos.y + size.y - (s.source_->getSample(i)-sMin) * yScale);
			ctx.shape->drawLine(prev, crt, uPos_.z, s.color_);
			prev = crt;
		}
		// draw value axis division lines & labels
		int nYDivs = size.y / yDivisionSize;

		pos.y += size.y + 15;
	}
}
