/*
 * results.h
 *
 *  Created on: Jul 23, 2016
 *      Author: alexandra
 */

#ifndef PERF_RESULTS_H_
#define PERF_RESULTS_H_

#include "../utils/MTVector.h"

class CallGraph;

namespace perf {

class Results {
	friend class CallGraph;
public:

	void getResults(); // TODO implement

private:
	static MTVector<CallGraph*> threadGraphs_;

	static void registerGraph(CallGraph &graph) {
		threadGraphs_.push_back(&graph);
	}
};

}

#endif /* PERF_RESULTS_H_ */
