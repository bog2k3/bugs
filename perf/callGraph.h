/*
 * callGraph.h
 *
 *  Created on: Aug 7, 2016
 *      Author: bog
 */

#ifndef PERF_CALLGRAPH_H_
#define PERF_CALLGRAPH_H_

#include "section.h"

#include <stack>
#include <unordered_map>
#include <memory>

namespace perf {

class CallGraph {
public:
	static void pushSection(const char name[], bool deadTime);
	static void popSection(uint64_t nanoseconds);

	static std::string getCrtThreadName() {
		return getCrtThreadInstance().threadName_;
	}

private:
	friend class Results;
	friend void setCrtThreadName(std::string name);

	CallGraph() {}
	static CallGraph& getCrtThreadInstance();

	struct charArrHash {
		size_t operator()(const char* s) const {
			size_t h = 5381;
			int c;
			const char* s0 = s;
			while ((c = *s0++))
				h = ((h << 5) + h) + c;
			return h;
		}
	};

	std::string threadName_;

	// this structure holds cummulated data for each section
	// (if a section is called from multiple other sections, all the timings here are aggregate)
	std::unordered_map<const char*, std::unique_ptr<sectionData>, charArrHash> flatSectionData_;

	// this holds call-tree data - a section with the same name may exist in multiple instances if called from different places
	std::vector<std::shared_ptr<sectionData>> rootTrees_;

	std::stack<sectionData*> crtStack_;

	static thread_local std::shared_ptr<CallGraph> crtThreadInstance_;
};

} // namespace

#endif /* PERF_CALLGRAPH_H_ */
