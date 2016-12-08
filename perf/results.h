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
class sectionData;

class Results {
	friend class CallGraph;
public:

	// return the number of threads that contain traced calls
	static unsigned getNumberOfThreads() { return threadGraphs_.size(); }

	static std::string getThreadName(unsigned id);

	// get a list of independent call trees on the specified thread
	static std::vector<std::shared_ptr<sectionData>> getCallTrees(unsigned threadID);
	// get a list of independent call trees on the specified named thread
	static std::vector<std::shared_ptr<sectionData>> getCallTrees(std::string const& threadName);
	// get a flat list of frames on the specified thread
	static std::vector<sectionData> getFlatList(unsigned threadID);
	// get a flat list of frames on the specified named thread
	static std::vector<sectionData> getFlatList(std::string const& threadName);

private:
	static MTVector<std::shared_ptr<CallGraph>> threadGraphs_;

	static void registerGraph(std::shared_ptr<CallGraph> graph) {
		threadGraphs_.push_back(graph);
	}
};

}

#endif /* PERF_RESULTS_H_ */
