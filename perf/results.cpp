/*
 * results.cpp
 *
 *  Created on: Jul 23, 2016
 *      Author: alexandra
 */

#include "results.h"
#include "callGraph.h"

#include <algorithm>

namespace perf {

MTVector<std::shared_ptr<CallGraph>> Results::threadGraphs_ { 16 };

std::string Results::getThreadName(unsigned id) {
	if (id > threadGraphs_.size())
		return "unknown thread";
	return threadGraphs_[id]->threadName_;
}

// get a list of independent call trees on the specified thread
std::vector<std::shared_ptr<sectionData>> Results::getCallTrees(unsigned threadID) {
	if (threadID >= threadGraphs_.size())
		return {};
	return threadGraphs_[threadID]->rootTrees_;
}

std::vector<std::shared_ptr<sectionData>> Results::getCallTrees(std::string const& threadName) {
	for (unsigned i=0; i<threadGraphs_.size(); i++)
		if (threadGraphs_[i]->threadName_ == threadName)
			return getCallTrees(i);
	return {};
}

// get a flat list of frames on the specified thread
std::vector<sectionData> Results::getFlatList(unsigned threadID) {
	if (threadID >= threadGraphs_.size())
		return {};
	std::vector<sectionData> ret;
	for (auto &p : threadGraphs_[threadID]->flatSectionData_)
		ret.push_back(*p.second);
	return ret;
}

std::vector<sectionData> Results::getFlatList(std::string const& threadName) {
	for (unsigned i=0; i<threadGraphs_.size(); i++)
		if (threadGraphs_[i]->threadName_ == threadName)
			return getFlatList(i);
	return {};
}

} // namespace perf

