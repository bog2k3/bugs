/*
 * main.cpp
 *
 *  Created on: Sep 2, 2016
 *      Author: bog
 */

#include <thread>
#include <cmath>
#include "perf/marker.h"
#include "perf/threadMarker.h"
#include "perf/results.h"

void foo() {
	perf::Marker(__FUNCTION__);
	for (int i=0; i<1000; i++)
		std::sqrt(i);
}

void callee(int rec) {
	perf::Marker(__FUNCTION__);
	if (rec)
		callee(rec-1);
	for (int i=0; i<10; i++)
		foo();
}

void caller(int rec) {
	perf::Marker(__FUNCTION__);
	callee(rec/2);
	if (rec)
		caller(rec-1);
}

void printTree(const std::vector<std::shared_ptr<perf::sectionData>> &t, int level) {
	const auto tab = "    ";
	for (auto &s : t) {
		for (int i=0; i<level; i++) {
			std::cout<<"|" << tab;
		}
		std::cout<<"|--" << s->getName() << tab << "[#" << s->getExecutionCount() << "  i:" << s->getInclusiveNanosec() << "us  e:" << s->getExclusiveNanosec() << "us]\n";
		printTree(s->getCallees(), level+1);
	}
}

int main() {
	perf::setCrtThreadName("main");
	{
		perf::Marker marker(__FUNCTION__);
		caller(4);
		perf::Marker marker2("part 2");
		foo();
	}

	auto res = perf::Results::getCallTrees(0);
	auto resn = perf::Results::getCallTrees("main");

	printTree(resn, 0);

	return 0;
}


