/*
 * frameCapture.cpp
 *
 *  Created on: Dec 17, 2016
 *      Author: bog
 */

#include "frameCapture.h"

#include <algorithm>

namespace perf {

std::atomic<FrameCapture::CaptureMode> FrameCapture::mode_ {FrameCapture::Disabled};
std::atomic<std::thread::id> FrameCapture::exclusiveThreadID_;
MTVector<std::shared_ptr<MTVector<FrameCapture::frameData>>> FrameCapture::allFrames_ {8};
MTVector<std::string> FrameCapture::threadNames_ {8};
std::chrono::time_point<std::chrono::high_resolution_clock> FrameCapture::captureStartTime_;
#ifdef DEBUG_INSTANCES
MTVector<FrameCapture*> FrameCapture::allInstances_ {8};
#endif

void FrameCapture::start(FrameCapture::CaptureMode mode) {
	assert(mode_.load(std::memory_order_consume) == Disabled && "Capture already in progress!");
	if (mode == ThisThreadOnly)
		exclusiveThreadID_.store(std::this_thread::get_id(), std::memory_order_release);
	captureStartTime_ = std::chrono::high_resolution_clock::now();
	mode_.store(mode, std::memory_order_release);
}

void FrameCapture::stop() {
	mode_.store(Disabled, std::memory_order_release);
	// check all unfinished frames
	auto now = std::chrono::high_resolution_clock::now();
	for (auto &tf : allFrames_)
		for (auto &f : *tf)
			if (f.endTime_.time_since_epoch().count() == 0)
				f.endTime_ = now;
}

std::string FrameCapture::getThreadNameForIndex(unsigned index) {
	assert(index < threadNames_.size());
	return threadNames_[index];
}

std::vector<FrameCapture::frameData> FrameCapture::getResults() {
	std::vector<FrameCapture::frameData> ret;
	for (auto &fv : allFrames_) {
		assert (fv != nullptr);
		fv->getContentsExclusive(ret);
	}
	std::sort(ret.begin(), ret.end(), [] (auto &x, auto &y) {
		return x.startTime_ < y.startTime_;
	});
	return ret;
}

void FrameCapture::cleanup() {
	assert(mode_ == Disabled && "Don't call this while capturing!!!");
	for (auto &fv : allFrames_) {
		fv->clear();
	}
}

} /* namespace perf */
