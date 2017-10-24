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
#include "../math/math3D.h"
#include "../utils/log.h"

#include <sstream>
#include <iomanip>

SignalDataSource::SignalDataSource(std::function<float()> getValue, int maxSamples, float sampleInterval)
	: getValue_(getValue), capacity_(maxSamples), sampleInterval_(sampleInterval) {
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
		samples_[n_++] = getValue_();
	else {
		samples_[zero_] = getValue_();
		zero_ = (zero_+1) % n_;
	}
}

SignalViewer::SignalViewer(ViewportCoord pos, float z, ViewportCoord size, std::string viewportFilter)
	: pos_(pos), z_(z), size_(size), viewportFilter_(viewportFilter) {
}

SignalViewer::~SignalViewer() {
}

void SignalViewer::addSignal(std::string const& name, float* pValue, glm::vec3 const& rgb, float sampleInterval, int maxSamples, float minUpperY, float maxLowerY, int displayPrecision) {
	addSignal(name, [pValue] { return *pValue; }, rgb, sampleInterval, maxSamples, minUpperY, maxLowerY, displayPrecision);
}

void SignalViewer::addSignal(std::string const& name, std::function<float()> getValue, glm::vec3 const& rgb, float sampleInterval, int maxSamples, float minUpperY, float maxLowerY, int displayPrecision) {
	sourceInfo_.push_back(DataInfo(std::unique_ptr<SignalDataSource>(new SignalDataSource(getValue, maxSamples, sampleInterval)), name, rgb, minUpperY, maxLowerY, displayPrecision));
}

void SignalViewer::update(float dt) {
	for (auto &s : sourceInfo_)
		s.source_->update(dt);
}

void SignalViewer::draw(RenderContext const&) {
	constexpr int maxYDivisions = 5;
	constexpr float textSize = 16;
	constexpr float spacePerChar = textSize/2; // pixels
	const glm::vec4 frameColor(1.f, 1.f, 1.f, 0.3f);
	const glm::vec4 divisionColor(1.f, 1.f, 1.f, 0.2f);
	const glm::vec4 divisionLabelColor(1.f, 1.f, 1.f, 0.6f);

	auto pos = pos_;

	Shape2D::get()->setViewportFilter(viewportFilter_);
	GLText::get()->setViewportFilter(viewportFilter_);

	for (auto &s : sourceInfo_) {
		Shape2D::get()->drawRectangle(pos, z_, size_, frameColor);
		float sMin = 1.e20f, sMax = -1.e20f;
		// scan all samples and seek min/max values:
		for (unsigned i=0; i<s.source_->getNumSamples(); i++) {
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
		if (sMin > s.maxLowerY_)
			sMin = s.maxLowerY_;
		if (sMax < s.minUpperY_)
			sMax = s.minUpperY_;
		// draw samples:
		ViewportCoord widthPerSample = size_.x() / s.source_->getCapacity();
		ViewportCoord pixelsPerYUnit = {0, 0};
		if (sMin != sMax)
			pixelsPerYUnit = size_.y() / (sMax - sMin);
		ViewportCoord prevVertex = pos + size_.y() * 0.5f;
		for (unsigned i=0; i<s.source_->getNumSamples(); i++) {
			ViewportCoord crtVertex = prevVertex.x() + widthPerSample +
					pos.y() + size_.y() - pixelsPerYUnit * (s.source_->getSample(i)-sMin);
			Shape2D::get()->drawLine(prevVertex, crtVertex, z_, s.color_);
			prevVertex = crtVertex;
		}
		// draw value axis division lines & labels
		if (sMin * sMax < 0) {
			// zero line is visible
			auto zeroY = pos.y() + size_.y() + pixelsPerYUnit*sMin;
			Shape2D::get()->drawLine(pos.x() + zeroY, pos.x() + size_.x() + zeroY, z_, frameColor);
		}
		int nYDivs = maxYDivisions; //min(maxYDivisions, (int)(size.y / yDivisionSize));
		int nDecimals = s.source_->getNumSamples() ? -log10(sMax - sMin) : 0;
		if (s.displayPrecision_ >= 0)
			nDecimals = s.displayPrecision_;
		ViewportCoord yDivisionSize = size_.y() / nYDivs;
		for (int i=1; i<nYDivs; i++) {
			auto lineY = pos.y() + size_.y() + yDivisionSize * (-i);
			Shape2D::get()->drawLine(pos.x() + lineY, pos.x() + size_.x() + lineY, z_, divisionColor);
			std::stringstream ss;
			ss << std::fixed << std::setprecision(nDecimals) << sMin + (sMax-sMin) * i / nYDivs;
			GLText::get()->print(ss.str(), pos.x() - ViewportCoord{(ss.str().size()+1) * spacePerChar, -textSize/2} + lineY, z_, textSize, divisionLabelColor);
		}
		// draw title and current value:
		std::stringstream stitle;
		stitle << s.name_;
		if (s.source_->getNumSamples())
			stitle << " : " << std::fixed << std::setprecision(nDecimals) << s.source_->getSample(s.source_->getNumSamples()-1);
		else
			stitle << " (no values)";
		GLText::get()->print(stitle.str(), pos, z_, textSize, s.color_);

		pos = pos + size_.y() + ViewportCoord{0, 10 + textSize};
	}

	Shape2D::get()->resetViewportFilter();
	GLText::get()->resetViewportFilter();
}
