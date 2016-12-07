/*
 * callGraph.cpp
 *
 *  Created on: Aug 7, 2016
 *      Author: bog
 */

#include "callGraph.h"
#include "results.h"

#include <cstring>
#include <algorithm>

namespace perf {

thread_local std::shared_ptr<CallGraph> CallGraph::crtThreadInstance_;

CallGraph& CallGraph::getCrtThreadInstance() {
	if (!crtThreadInstance_) {
		crtThreadInstance_.reset(new CallGraph());
		Results::registerGraph(crtThreadInstance_);
	}
	return *crtThreadInstance_;
}

void CallGraph::pushSection(const char name[]) {
	// add to call-trees:
	std::vector<std::shared_ptr<sectionData>> &treeContainer =
		getCrtThreadInstance().crtStack_.empty()
			? getCrtThreadInstance().rootTrees_
			: getCrtThreadInstance().crtStack_.top()->callees_;
	auto treeIt = std::find_if(treeContainer.begin(), treeContainer.end(), [&name] (auto &sec) {
		return !std::strcmp(sec->name_, name);
	});
	if (treeIt == treeContainer.end()) {
		treeIt = treeContainer.emplace_back(std::make_shared<sectionData>(name));
	}
	getCrtThreadInstance().crtStack_.push(treeIt->get());

	// add to flat list:
	auto &flatList = getCrtThreadInstance().flatSectionData_;

	auto it = flatList.find(name);
	if (it == flatList.end()) {
		it = flatList.emplace(name, std::unique_ptr<sectionData>(new sectionData(name))).first;
	}
}

void CallGraph::popSection(unsigned nanoseconds) {
	auto &stack = getCrtThreadInstance().crtStack_;
	// add time to secion, ++callCount
	sectionData *pCrt = stack.top();
	pCrt->executionCount_++;
	pCrt->nanoseconds_ += nanoseconds;
	stack.pop();

	if (stack.empty())
		return;

	// increment previous->crt_frame edge time & callcount
	sectionData* pPrev = stack.top();
	auto &edges = getCrtThreadInstance().edges_;
	auto edgeName = std::make_pair(pPrev->name_, pCrt->name_);
	auto it = edges.find(edgeName);
	if (it == edges.end())
		it = edges.emplace(edgeName, EdgeData()).first;
	it->second.totalNanoseconds_ += nanoseconds;
	it->second.callCount_++;
}

} // namespace
