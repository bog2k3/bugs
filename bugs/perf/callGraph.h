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
	class Edge {
		friend class CallGraph;
	private:
		char caller_[256];
		char callee_[256];
		unsigned totalNanoseconds_ = 0;
		unsigned callCount_ = 0;
	};

	CallGraph();

	static void pushSection(const char name[]);
	static void popSection(unsigned nanoseconds);

private:
	struct charArrHash
	{
		size_t operator()(const char* s) const
		{
			size_t h = 5381;
			int c;
			const char* s0 = s;
			while ((c = *s0++))
				h = ((h << 5) + h) + c;
			return h;
		}
	};

	std::unordered_map<const char*, std::unique_ptr<sectionData>, charArrHash> sections_;
	std::stack<sectionData*> crtStack_;

	static thread_local CallGraph crtInstance_;
};

}

#endif /* PERF_CALLGRAPH_H_ */
