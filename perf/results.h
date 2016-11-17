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
#include <vector>

namespace perf {

class CallGraph;

class Results {
	friend class CallGraph;
public:

	struct CallFrame {
		std::string name;
		unsigned inclusiveNanosec;
		unsigned exclusiveNanosec;
		unsigned callCount;
		CallFrame(std::string &&name, unsigned inclusiveNs, unsigned exclusiveNs, unsigned callCount)
			: name(std::move(name)), inclusiveNanosec(inclusiveNs), exclusiveNanosec(exclusiveNs), callCount(callCount)
		{}
		CallFrame(CallFrame &&f) : CallFrame(std::move(f.name), f.inclusiveNanosec, f.exclusiveNanosec, f.callCount) {}
	};

	struct CallTree {
		const CallFrame nodeFrame;
		std::vector<std::shared_ptr<CallTree>> callees;
		CallTree(CallFrame &&nodeFrame) : nodeFrame(std::move(nodeFrame)) {}
	};

	// return the number of threads that contain traced calls
	static unsigned getNumberOfThreads() { return threadGraphs_.size(); }

	// get a list of independent call trees on the specified thread
	static std::vector<std::shared_ptr<CallTree>> getCallTree(unsigned threadID);  // TODO
	// get a list of independent call trees on the specified named thread
	static std::vector<std::shared_ptr<CallTree>> getCallTree(std::string const& threadName);  // TODO
	// get a flat list of frames on the specified thread
	static std::vector<std::shared_ptr<CallFrame>> getFlatList(unsigned threadID); // TODO

	// TODO: have a way to name threads - maybe a ThreadMarker("walala") which would assign a string to the threadID
	// TODO: void ProcessData() which construct the trees and frames internally, ready for inspection - called internally first time needed
	// TODO: assert(crt thread's callStack is empty) when process() is called

private:
	static MTVector<std::shared_ptr<CallGraph>> threadGraphs_;

	static void registerGraph(std::shared_ptr<CallGraph> graph) {
		threadGraphs_.push_back(graph);
	}

	static void processData();
};

}

#endif /* PERF_RESULTS_H_ */
