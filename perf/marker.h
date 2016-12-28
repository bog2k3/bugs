/*
 * marker.h
 *
 *  Created on: Jul 22, 2016
 *      Author: bog
 */

#ifndef PERF_MARKER_H_
#define PERF_MARKER_H_

#include "callGraph.h"
#include "frameCapture.h"

#include <chrono>

#define ENABLE_PERF_MARKERS

#ifdef ENABLE_PERF_MARKERS
	#define PERF_MARKER_FUNC perf::Marker funcMarker##__LINE__(__PRETTY_FUNCTION__)
	#define PERF_MARKER_FUNC_BLOCKED perf::Marker funcMarker##__LINE__(__PRETTY_FUNCTION__, true)
	#define PERF_MARKER(NAME) perf::Marker perfMarker##__LINE__(NAME)
	#define PERF_MARKER_BLOCKED(NAME) perf::Marker perfMarker##__LINE__(NAME, true)
#else
	#define PERF_MARKER_FUNC
	#define PERF_MARKER_FUNC_BLOCKED
	#define PERF_MARKER(NAME)
	#define PERF_MARKER_BLOCKED(NAME)
#endif

namespace perf {

inline void setCrtThreadName(std::string name) {
	CallGraph::getCrtThreadInstance().threadName_ = name;
}

#define DEBUG_MARKERS

class Marker {
public:
	Marker(const char name[], bool blocked = false) {
		CallGraph::pushSection(name, blocked);
		start_ = std::chrono::high_resolution_clock::now();
		if (FrameCapture::captureEnabledOnThisThread()) {
#ifdef DEBUG_MARKERS
			wasCapture_ = true;
			strncpy(name_, name, sizeof(name_));
			std::cout << "MARKER START :: " << name << "\n";
#endif
			FrameCapture::beginFrame(name, start_, blocked);
		}
	}

	~Marker() {
		auto end = std::chrono::high_resolution_clock::now();
		auto nanosec = std::chrono::nanoseconds(end - start_).count();
		CallGraph::popSection(nanosec);
		if (FrameCapture::captureEnabledOnThisThread()) {
#ifdef DEBUG_MARKERS
			std::cout << "MARKER END   :: " << name_ << "\n";
#endif
			FrameCapture::endFrame(end);
		}
#ifdef DEBUG_MARKERS
		else if (wasCapture_) {
			std::cout << "!!!! ERROR MARKER END :: " << name_ << "\n";
		}
#endif
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;
#ifdef DEBUG_MARKERS
	bool wasCapture_ = false;
	char name_[256];
#endif
};

} // namespace perf

#endif /* PERF_MARKER_H_ */
