/*
 * callGraph.cpp
 *
 *  Created on: Aug 7, 2016
 *      Author: bog
 */

#include "callGraph.h"

namespace perf {

thread_local CallGraph CallGraph::crtInstance_;

void CallGraph::pushSection(const char name[]) {
	auto &sections = crtInstance_.sections_;
	auto &crtStack = crtInstance_.crtStack_;

	auto it = sections.find(name);
	if (it == sections.end()) {
		it = sections.emplace(name, std::unique_ptr<sectionData>(new sectionData(name))).first;
	}
	crtStack.push(it->second.get());
}

void CallGraph::popSection(unsigned nanoseconds) {
	// TODO: add time to secion, ++callCount, increment previous->crt_frame edge time & callcount
}

} // namespace
