/*
 * results.cpp
 *
 *  Created on: Jul 23, 2016
 *      Author: alexandra
 */

#include "results.h"
#include "callGraph.h"

namespace perf {

MTVector<std::shared_ptr<CallGraph>> Results::threadGraphs_ { 16 };

// get a list of independent call trees on the specified thread
std::vector<std::shared_ptr<Results::CallTree>> Results::getCallTree(unsigned threadID) {
	if (threadID >= threadGraphs_.size())
		return {nullptr};
	auto pGraph = threadGraphs_[threadID];
	pGraph->edges_;
}
// get a flat list of frames on the specified thread
std::vector<std::shared_ptr<Results::CallFrame>> Results::getFlatList(unsigned threadID) {

}

}

