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

template<class ITER, class F>
void parallel_for(ITER itB, ITER itE, ThreadPool &pool, F predicate)
{
	size_t rangeSize = std::distance(itB, itE);
	uint chunks = std::min((size_t)pool.getThreadCount(), rangeSize);
	uint chunkSize = std::max((size_t)1, rangeSize / chunks);

	std::vector<PoolTaskHandle> tasks;
	decltype(itB) start = itB;
	for (uint i=0; i<chunks; i++) {
		tasks.push_back(pool.queueTask([start, chunkSize, predicate] () mutable {
			for (uint k=0; k<chunkSize; k++, ++start)
				predicate(*start);
		}));
		start += chunkSize;
	}
	// wait for pool tasks to finish:
	for (auto &t : tasks)
		t->wait();
}


#endif /* UTILS_PARALLEL_H_ */
