/*
 * callGraph.cpp
 *
 *  Created on: Aug 7, 2016
 *      Author: bog
 */

#include "callGraph.h"
#include "results.h"

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
	auto &sections = getCrtThreadInstance().sections_;
	auto &crtStack = getCrtThreadInstance().crtStack_;

	auto it = sections.find(name);
	if (it == sections.end()) {
		it = sections.emplace(name, std::unique_ptr<sectionData>(new sectionData(name))).first;
	}
	crtStack.push(it->second.get());
}

void CallGraph::popSection(unsigned nanoseconds) {
	auto &stack = getCrtThreadInstance().crtStack_;
	// add time to secion, ++callCount
	sectionData *pCrt = stack.top();
	pCrt->executionCount_++;
	pCrt->nanoseconds_ += nanoseconds;
	stack.pop();

	// increment previous->crt_frame edge time & callcount
	sectionData* pPrev = stack.top();
	auto &edges = getCrtThreadInstance().edges_;
	auto edgeName = std::make_pair(pPrev->name_, pCrt->name_);
	auto it = edges.find(edgeName);
	if (it == edges.end())
		it = edges.emplace(edgeName, Edge()).first;
	it->second.totalNanoseconds_ += nanoseconds;
	it->second.callCount_++;
}

} // namespace
