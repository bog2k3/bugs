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

#include <sstream>
#include <iomanip>

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

void SignalViewer::addSignal(std::string const& name, float* pValue, glm::vec3 const& rgb, float sampleInterval, int maxSamples, float minYScale) {
	sourceInfo_.push_back(DataInfo(std::unique_ptr<SignalDataSource>(new SignalDataSource(pValue, maxSamples, sampleInterval)), name, rgb, minYScale));
}

void SignalViewer::update(float dt) {
	for (auto &s : sourceInfo_)
		s.source_->update(dt);
}

void SignalViewer::draw(RenderContext const& ctx) {
	constexpr float yDivisionSize = 20; // pixels
	constexpr int maxYDivisions = 5;
	constexpr float textSize = 14;
	constexpr float spacePerChar = textSize/2; // pixels
	const glm::vec4 frameColor(1.f, 1.f, 1.f, 0.3f);
	const glm::vec4 divisionColor(1.f, 1.f, 1.f, 0.2f);
	const glm::vec4 divisionLabelColor(1.f, 1.f, 1.f, 0.6f);

	glm::vec2 pos = vec3xy(uPos_);
	pos.x *= ctx.viewport->getWidth();
	pos.y *= ctx.viewport->getHeight();
	glm::vec2 size = uSize_;
	size.x *= ctx.viewport->getWidth();
	size.y *= ctx.viewport->getHeight();
	for (auto &s : sourceInfo_) {
		ctx.shape->setViewportSpaceDraw(true);
		ctx.shape->drawRectangle(pos, uPos_.z, size, frameColor);
		ctx.text->print(s.name_, pos.x, pos.y, uPos_.z, textSize, s.color_);
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
		if (sMin == sMax)
			yScale = 0;
		if (yScale < s.minYScale_)
			yScale = s.minYScale_;
		glm::vec2 prev(pos.x, pos.y + size.y * 0.5f);
		for (uint i=0; i<s.source_->getNumSamples(); i++) {
			glm::vec2 crt(prev.x + xAxisZoom, pos.y + size.y - (s.source_->getSample(i)-sMin) * yScale);
			ctx.shape->drawLine(prev, crt, uPos_.z, s.color_);
			prev = crt;
		}
		// draw value axis division lines & labels
		if (sMin * sMax < 0) {
			// zero line is visible
			float zeroY = pos.y+size.y + sMin*yScale;
			ctx.shape->drawLine(glm::vec2(pos.x, zeroY), glm::vec2(pos.x+size.x, zeroY), uPos_.z, frameColor);
		}
		int nYDivs = min(maxYDivisions, (int)(size.y / yDivisionSize));
		int nDecimals = s.source_->getNumSamples() ? -log10(sMax - sMin) : 0;
		for (int i=1; i<nYDivs; i++) {
			float lineY = pos.y + size.y - i * yDivisionSize;
			ctx.shape->drawLine(glm::vec2(pos.x, lineY), glm::vec2(pos.x+size.x, lineY), uPos_.z, divisionColor);
			std::stringstream ss;
			ss << std::setprecision(nDecimals) << sMin + (sMax-sMin) * i / nYDivs;
			ctx.text->print(ss.str(), pos.x - (ss.str().size()+1) * spacePerChar, lineY+5, uPos_.z, textSize, divisionLabelColor);
		}

		pos.y += size.y + 15;
	}
}
