/*
 * frameCapture.h
 *
 *  Created on: Dec 17, 2016
 *      Author: bog
 */

#ifndef PERF_FRAMECAPTURE_H_
#define PERF_FRAMECAPTURE_H_

#include "callGraph.h"

#include "../utils/MTVector.h"

#include <vector>
#include <thread>
#include <chrono>
#include <cassert>
#include <cstring>
#include <atomic>
#include <memory>

namespace perf {

class FrameCapture {
public:
	enum CaptureMode {
		Disabled,
		ThisThreadOnly,
		AllThreads,
	};

	struct frameData {
		std::chrono::time_point<std::chrono::high_resolution_clock> startTime_;
		std::chrono::time_point<std::chrono::high_resolution_clock> endTime_;
		char name_[256];
		unsigned threadIndex_;

		frameData(const char name[], std::chrono::time_point<std::chrono::high_resolution_clock> start, unsigned threadIndex)
			: startTime_(start), threadIndex_(threadIndex) {
			strncpy(name_, name, sizeof(name_)/sizeof(name_[0]));
		}
	};

	// start capturing a frame, recording all markers' absolute times
	static void start(CaptureMode mode);
	// stop capturing the frame
	static void stop();

	// aggregates all calls from all recorded threads (depending on capture mode) and returns a linear sequence
	// ordered chronologically, with interleaved threads
	static std::vector<frameData> getResults();

	// returns the name of the thread identified by index (use index from frameData)
	static std::string getThreadNameForIndex(unsigned index);

	// call this to clean up all data and start fresh next time
	static void cleanup();

private:
	friend class Marker;

	static bool captureEnabledOnThisThread() {
		if (mode_ == Disabled)
			return false;
		return (mode_ == AllThreads || std::this_thread::get_id() == exclusiveThreadID_);
	}

	static void beginFrame(const char name[], std::chrono::time_point<std::chrono::high_resolution_clock> now) {
		getThreadInstance().frames_->emplace_back(name, now, getThreadInstance().threadIndex_);
		getThreadInstance().frameStack_.push(&getThreadInstance().frames_->back());
	}

	static void endFrame(std::chrono::time_point<std::chrono::high_resolution_clock> now) {
		if (!getThreadInstance().frameStack_.size())
			beginFrame("{UNKNOWN}", captureStartTime_);
		getThreadInstance().frameStack_.top()->endTime_ = now;
		getThreadInstance().frameStack_.pop();
	}

	static std::atomic<CaptureMode> mode_;
	static std::atomic<std::thread::id> exclusiveThreadID_;
	static MTVector<std::shared_ptr<std::vector<frameData>>> allFrames_;
	static MTVector<std::string> threadNames_;
	static std::chrono::time_point<std::chrono::high_resolution_clock> captureStartTime_;

	std::shared_ptr<std::vector<frameData>> frames_;
	std::stack<frameData*> frameStack_;
	unsigned threadIndex_;
	static FrameCapture& getThreadInstance() {
		static thread_local FrameCapture instance;
		return instance;
	}
	FrameCapture() : frames_(new std::vector<frameData>()) {
		allFrames_.push_back(frames_);
		threadIndex_ = threadNames_.push_back(CallGraph::getCrtThreadName());
	}
};

} /* namespace perf */

#endif /* PERF_FRAMECAPTURE_H_ */
