/*
 * SignalViewer.cpp
 *
 *  Created on: Jan 24, 2016
 *      Author: bog
 */

#include "SignalViewer.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
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

void SignalViewer::addSignal(std::string const& name, float* pValue, int maxSamples, float sampleInterval, float xAxisZoom, glm::vec3 const& rgb) {
	sourceInfo_.push_back(DataInfo(std::unique_ptr<SignalDataSource>(new SignalDataSource(pValue, maxSamples, sampleInterval)), name, xAxisZoom, rgb));
}

void SignalViewer::update(float dt) {
	for (auto &s : sourceInfo_)
		s.source_->update(dt);
}

void SignalViewer::draw(RenderContext const& ctx) {
	glm::vec2 pos = vec3xy(uPos_);
	pos.x *= ctx.viewport->getWidth();
	pos.y *= ctx.viewport->getHeight();
	glm::vec2 size = uSize_;
	size.x *= ctx.viewport->getWidth();
	size.y *= ctx.viewport->getHeight();
	for (auto &s : sourceInfo_) {
		ctx.shape->setViewportSpaceDraw(true);
		ctx.shape->drawRectangle(pos, uPos_.z, size, s.color_);

		pos.y += uSize_.y * 1.05f;
	}
}
