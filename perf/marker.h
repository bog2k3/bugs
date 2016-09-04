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

namespace perf {

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
