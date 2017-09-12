/*
 * ThreadPool.h
 *
 *  Created on: Jun 21, 2016
 *      Author: bog
 */

#ifndef UTILS_THREADPOOL_H_
#define UTILS_THREADPOOL_H_

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <thread>

//#define DEBUG_THREADPOOL	// to enable debug logs

#ifdef DEBUG_THREADPOOL
#include "../../bugs/utils/log.h"
#endif


class PoolTask {
public:
	void wait();
	bool isFinished() { return finished_; }
private:
	std::mutex workMutex_;
	std::atomic<bool> started_ { false };
	std::atomic<bool> finished_ { false };
	std::function<void()> workFunc_;

	friend class ThreadPool;
	PoolTask(decltype(workFunc_) func)
		: workFunc_(func) {
	}
};
using PoolTaskHandle = std::shared_ptr<PoolTask>;

class ThreadPool {
public:
	ThreadPool(unsigned numberOfThreads);
	~ThreadPool();	// make sure you call stop() before destruction

	void stop(); // waits for all tasks to be processed, waits for all workers to finish and shuts down the threads in the pool

	template<class F, class... Args>
	PoolTaskHandle queueTask(F task, Args... args) {
		std::unique_lock<std::mutex> lk(poolMutex_);
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " mutex acquired.");
#endif
		checkValidState();
		auto handle = std::shared_ptr<PoolTask>(new PoolTask([=] () mutable { task(args...); }));
		queuedTasks_.push(handle);
		lk.unlock();
		condPendingTask_.notify_one();
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " notify_one()");
#endif
		return handle;
	}

	unsigned getThreadCount() const { return workers_.size(); }

protected:
	std::queue<PoolTaskHandle> queuedTasks_;
	std::mutex poolMutex_;
	std::condition_variable condPendingTask_;
	std::vector<std::thread> workers_;
	std::atomic<bool> stopSignal_ { false };	// signal workers to stop
	std::atomic<bool> stopRequested_ { false };	// stop requested by user
	std::atomic<bool> stopped_ { false };

	void workerFunc();

	void checkValidState();
};



#endif /* UTILS_THREADPOOL_H_ */
