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

// get a list of independent call trees on the specified thread
std::vector<std::shared_ptr<Results::CallTree>> Results::getCallTrees(unsigned threadID) {
	if (threadID >= threadGraphs_.size())
		return {};
	if (threadTrees_.empty())
		processData();
	std::vector<std::shared_ptr<CallTree>> ret;
	for (auto &p : threadTrees_) {
		if (p.first == threadID)
			ret.push_back(p.second);
	}
	return ret;
}

std::vector<std::shared_ptr<Results::CallTree>> Results::getCallTrees(std::string const& threadName) {
	for (unsigned i=0; i<threadGraphs_.size(); i++)
		if (threadGraphs_[i]->threadName_ == threadName)
			return getCallTrees(i);
	return {};
}

// get a flat list of frames on the specified thread
std::vector<Results::CallFrame> Results::getFlatList(unsigned threadID) {
	std::vector<CallFrame> ret;
	if (threadTrees_.empty())
		processData();
	for (auto &p : threadTrees_) {
		if (p.first != threadID)
			continue;
		addNodeDataToListRecursive(ret, *p.second);
	}
	return ret;
}

std::vector<Results::CallFrame> Results::getFlatList(std::string const& threadName) {
	for (unsigned i=0; i<threadGraphs_.size(); i++)
		if (threadGraphs_[i]->threadName_ == threadName)
			return getFlatList(i);
	return {};
}

void Results::addNodeDataToListRecursive(std::vector<CallFrame> &list, CallTree const& node) {
	auto it = std::find_if(list.begin(), list.end(), [&node](auto &x) {
		return x.name == node.nodeFrame.name;
	});
	if (it != list.end())
		*it += node.nodeFrame;
	else
		list.push_back(node.nodeFrame);
	for (auto &callee : node.callees)
		addNodeDataToListRecursive(list, *callee);
}

void Results::processData() {
	if (!graph->crtStack_.empty())
		throw std::runtime_error("There are still functions executing while trying to compute results!!!");
}

} // namespace perf

