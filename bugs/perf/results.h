/*
 * results.h
 *
 *  Created on: Jul 23, 2016
 *      Author: alexandra
 */

#ifndef PERF_RESULTS_H_
#define PERF_RESULTS_H_

#include "../utils/MTVector.h"
#include <memory>

class CallGraph;

namespace perf {

class Results {
	friend class CallGraph;
public:

	void getResults(); // TODO implement

private:
	static MTVector<std::shared_ptr<CallGraph>> threadGraphs_;

	static void registerGraph(std::shared_ptr<CallGraph> graph) {
		threadGraphs_.push_back(graph);
	}
};

}

#endif /* PERF_RESULTS_H_ */
