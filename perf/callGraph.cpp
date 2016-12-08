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
		treeContainer.emplace_back(sectionData::create(name));
		treeIt = treeContainer.end()-1;
	}
	getCrtThreadInstance().crtStack_.push(treeIt->get());
}

void CallGraph::popSection(unsigned nanoseconds) {
	auto &stack = getCrtThreadInstance().crtStack_;
	// add time to secion, ++callCount
	sectionData *pCrt = stack.top();
	pCrt->executionCount_++;
	pCrt->nanoseconds_ += nanoseconds;
	stack.pop();

	// add time to flat list:
	auto &flatList = getCrtThreadInstance().flatSectionData_;

	auto it = flatList.find(pCrt->name_);
	if (it == flatList.end()) {
		it = flatList.emplace(pCrt->name_, std::unique_ptr<sectionData>(new sectionData(pCrt->name_))).first;
	}
	it->second->executionCount_++;
	it->second->nanoseconds_ += nanoseconds;
}

} // namespace
