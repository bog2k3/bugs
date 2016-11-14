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
	friend class Results;
public:

	class Edge {
		friend class CallGraph;
		friend class Results;
	private:
		unsigned totalNanoseconds_ = 0;
		unsigned callCount_ = 0;
	};

	static void pushSection(const char name[]);
	static void popSection(unsigned nanoseconds);

private:
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
	struct namePairHash {
		size_t operator() (std::pair<const char*, const char*> p) const {
			charArrHash hasher;
			auto x = hasher(p.first);
			auto y = hasher(p.second);
			return y + 0x9e3779b9U + (x << 6) + (x >> 2);
		}
	};

	std::unordered_map<const char*, std::unique_ptr<sectionData>, charArrHash> sections_;
	std::stack<sectionData*> crtStack_;
	std::unordered_map<std::pair<const char*, const char*>, Edge, namePairHash> edges_;

	static thread_local std::shared_ptr<CallGraph> crtThreadInstance_;
};

} // namespace

#endif /* PERF_CALLGRAPH_H_ */
