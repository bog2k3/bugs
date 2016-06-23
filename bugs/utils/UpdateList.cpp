/*
 * UpdateList.cpp
 *
 *  Created on: Jun 18, 2016
 *      Author: bog
 */

#include "UpdateList.h"
#include "../utils/ThreadPool.h"
#include <algorithm>
#include <array>

UpdateList::UpdateList(uint parallelThreads)
	: parallelThreadCount_(parallelThreads)
{
	if (parallelThreads > 1)
		pThreadPool = new ThreadPool(parallelThreads);
}

UpdateList::~UpdateList() {
	if (pThreadPool) {
		pThreadPool->stop();
		delete pThreadPool;
	}
}

void UpdateList::update(float dt) {
	// add pending:
	std::move(pendingAdd_.begin(), pendingAdd_.end(), std::back_inserter(list_));
	pendingAdd_.clear();
	// remove pending:
	list_.erase(std::remove_if(list_.begin(), list_.end(), [this] (const updatable_wrap& x) {
		for (void* ptr : pendingRemove_)
			if (x.equal_raw(ptr))
				return true;
		return false;
	}), list_.end());
	pendingRemove_.clear();
	// do update on current elements:
	// split list into parallelThreadCount_ subsets and run them on separate threads
	// use a thread pool for this
	uint chunks = std::min((size_t)parallelThreadCount_, list_.size());
	uint chunkSize = list_.size() / chunks;

	if (chunks > 1) {
		// use thread pool
		std::vector<PoolTaskHandle> tasks;
		uint start = 0;
		for (uint i=0; i<chunks-1; i++) {
			tasks.push_back(pThreadPool->queueTask([this, start, chunkSize, dt] {
				for (uint k=start; k<start+chunkSize; k++)
					list_[k].update(dt);
			}));
			start += chunkSize;
		}
		// run the last chunk on this thread:
		for (uint k=start; k<list_.size(); k++)
			list_[k].update(dt);
		// wait for other threads to finish:
		for (auto &t : tasks)
			t->wait();
	} else {
		// run them on this thread
		for (auto &e : list_)
			e.update(dt);
	}
}

