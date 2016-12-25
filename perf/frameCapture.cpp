/*
 * frameCapture.cpp
 *
 *  Created on: Dec 17, 2016
 *      Author: bog
 */

#include "frameCapture.h"

namespace perf {

std::atomic<FrameCapture::CaptureMode> FrameCapture::mode_ {FrameCapture::Disabled};
std::atomic<std::thread::id> FrameCapture::exclusiveThreadID_;
MTVector<std::shared_ptr<std::vector<FrameCapture::frameData>>> FrameCapture::allFrames_ {8};
MTVector<std::string> FrameCapture::threadNames_ {8};

std::string FrameCapture::getThreadNameForIndex(unsigned index) {
	assert(index < threadNames_.size());
	return threadNames_[index];
}

std::vector<FrameCapture::frameData> FrameCapture::getResults() {

}

void FrameCapture::cleanup() {
	assert(mode_ == Disabled && "Don't call this while capturing!!!");
	for (auto &fv : allFrames_) {
		fv->clear();
	}
}

} /* namespace perf */
