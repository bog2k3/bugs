/*
 * marker.h
 *
 *  Created on: Jul 22, 2016
 *      Author: bog
 */

#ifndef PERF_MARKER_H_
#define PERF_MARKER_H_

#include <chrono>

namespace perf {

class Marker {
public:
	Marker(const char name[]) {
		strncpy(name_, name, sizeof(name_)/sizeof(name_[0]));
		start_ = std::chrono::high_resolution_clock::now();
	}

	~Marker() {
		auto end = std::chrono::high_resolution_clock::now();
		auto nanosec = std::chrono::nanoseconds(end - start);
		// TODO talk to stack here
	}

private:
	char name_[256];
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

}

#endif /* PERF_MARKER_H_ */
