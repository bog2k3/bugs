/*
 * parallel.h
 *
 *  Created on: Jun 22, 2016
 *      Author: bog
 */

#ifndef UTILS_PARALLEL_H_
#define UTILS_PARALLEL_H_

#include "ThreadPool.h"

#include <iterator>
#include <algorithm>
#include <vector>

static constexpr size_t maxItemsPerJob = 8;

template<class ITER, class F>
void parallel_for(ITER itB, ITER itE, ThreadPool &pool, F predicate)
{
	size_t rangeSize = std::distance(itB, itE);
	if (rangeSize == 0)
		return;
	unsigned minJobs = std::min((size_t)pool.getThreadCount(), rangeSize);
	unsigned itemsPerJob = std::min(maxItemsPerJob, rangeSize / minJobs);
	unsigned jobs = rangeSize / itemsPerJob;
	if (jobs * itemsPerJob < rangeSize)
		++jobs;

	std::vector<PoolTaskHandle> tasks;
	decltype(itB) start = itB;

	for (unsigned i=0; i<jobs; ++i) {
		if (i == jobs-1) {
			// last job - make sure we don't lose any remaining elements:
			itemsPerJob = rangeSize - (jobs-1)*itemsPerJob;
		}
		tasks.push_back(pool.queueTask([start, itemsPerJob, predicate] () mutable {
			for (unsigned k=0; k<itemsPerJob; k++, ++start)
				predicate(*start);
		}));
		start += itemsPerJob;
	}
	// wait for pool tasks to finish:
	for (auto &t : tasks)
		t->wait();
}


#endif /* UTILS_PARALLEL_H_ */
