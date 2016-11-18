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
		CallFrame(CallFrame const& f) = default;

		CallFrame& operator += (CallFrame const& x) {
			assert(name == x.name);
			inclusiveNanosec += x.inclusiveNanosec;
			exclusiveNanosec += x.exclusiveNanosec;
			callCount += x.callCount;
			return *this;
		}
	};

	struct CallTree {
		const CallFrame nodeFrame;
		std::vector<std::shared_ptr<CallTree>> callees;
		CallTree(CallFrame nodeFrame) : nodeFrame(std::move(nodeFrame)) {}
	};

	// return the number of threads that contain traced calls
	static unsigned getNumberOfThreads() { return threadGraphs_.size(); }

	// get a list of independent call trees on the specified thread
	static std::vector<std::shared_ptr<CallTree>> getCallTrees(unsigned threadID);
	// get a list of independent call trees on the specified named thread
	static std::vector<std::shared_ptr<CallTree>> getCallTrees(std::string const& threadName);
	// get a flat list of frames on the specified thread
	static std::vector<CallFrame> getFlatList(unsigned threadID);
	// get a flat list of frames on the specified named thread
	static std::vector<CallFrame> getFlatList(std::string const& threadName);

private:
	static MTVector<std::shared_ptr<CallGraph>> threadGraphs_;

	static void registerGraph(std::shared_ptr<CallGraph> graph) {
		threadGraphs_.push_back(graph);
	}

	// first is threadID, second is a CallTree from that thread
	static std::vector<std::pair<unsigned, std::shared_ptr<CallTree>>> threadTrees_;

	static void processData();
	static void addNodeDataToListRecursive(std::vector<CallFrame> &list, CallTree const& node);
};

}

#endif /* PERF_RESULTS_H_ */
