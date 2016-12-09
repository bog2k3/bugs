/*
 * marker.h
 *
 *  Created on: Jul 22, 2016
 *      Author: bog
 */

#ifndef PERF_MARKER_H_
#define PERF_MARKER_H_

#include "callGraph.h"

#include <chrono>
#include <cstring>

#define ENABLE_PERF_MARKERS

#ifdef ENABLE_PERF_MARKERS
	#define PERF_MARKER_FUNC perf::Marker funcMarker##__LINE__(__PRETTY_FUNCTION__)
	#define PERF_MARKER(NAME) perf::Marker perfMarker##__LINE__(NAME)
#else
	#define PERF_MARKER_FUNC
	#define PERF_MARKER(NAME)
#endif

namespace perf {

inline void setCrtThreadName(std::string name) {
	CallGraph::getCrtThreadInstance().threadName_ = name;
}

class Marker {
public:
	Marker(const char name[]) {
		CallGraph::pushSection(name);
		start_ = std::chrono::high_resolution_clock::now();
	}

	~Marker() {
		auto end = std::chrono::high_resolution_clock::now();
		auto nanosec = std::chrono::nanoseconds(end - start_).count();
		CallGraph::popSection(nanosec);
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

}

#endif /* PERF_MARKER_H_ */