/*
 * section.h
 *
 *  Created on: Jul 22, 2016
 *      Author: bog
 */

#ifndef PERF_SECTION_H_
#define PERF_SECTION_H_

#include <cstring>
#include <vector>
#include <memory>
#include <numeric>

namespace perf {

class sectionData {
public:
	std::string getName() const { return name_; }
	unsigned getInclusiveNanosec() const { return nanoseconds_; }
	unsigned getExclusiveNanosec() const { return nanoseconds_ - std::accumulate(callees_.begin(), callees_.end(), 0,
			[] (auto sum, auto &callee) {
			return sum + callee->nanoseconds_;
		});
	}
	unsigned getExecutionCount() const { return executionCount_; }
	const std::vector<std::shared_ptr<sectionData>>& getCallees() const { return callees_; }

private:
	friend class CallGraph;

	static std::shared_ptr<sectionData> create(const char name[]) {
		return std::shared_ptr<sectionData>(new sectionData(name));
	}

	sectionData(const char name[]) {
		strncpy(name_, name, sizeof(name_)/sizeof(name_[0]));
	}

	unsigned nanoseconds_ = 0;
	unsigned executionCount_ = 0;
	char name_[256];
	std::vector<std::shared_ptr<sectionData>> callees_;
};

}

#endif /* PERF_SECTION_H_ */
