/*
 * ThreadPool.cpp
 *
 *  Created on: Jun 21, 2016
 *      Author: bog
 */

#include "ThreadPool.h"
#include "../perf/marker.h"
#include "assert.h"

ThreadPool::ThreadPool(uint numberOfThreads)
{
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__);
#endif
	for (uint i=0; i<numberOfThreads; i++)
		workers_.push_back(std::thread(std::bind(&ThreadPool::workerFunc, this)));
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " finished.");
#endif
}
ThreadPool::~ThreadPool() {	// throws exception if any thread is currently working - make sure you call stop() before destruction
	assertDbg(stopped_ && "Thread pool has not been stopped before destruction!");
}

void ThreadPool::stop() {
	// wait for all tasks to be processed
	std::unique_lock<std::mutex> poolLk(poolMutex_);
	checkValidState();
	stopRequested_.store(true);
	while (!queuedTasks_.empty()) {
		poolLk.unlock();
		std::this_thread::yield();
		poolLk.lock();
	}
	// wait for all workers to finish and shuts down the threads in the pool
	stopSignal_.store(true);
	poolLk.unlock();
	condPendingTask_.notify_all();
	for (auto &t : workers_)
		t.join();
	stopped_.store(true);
}

void ThreadPool::workerFunc() {
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " begin");
#endif
	perf::setCrtThreadName("ThreadPool::worker");
	PERF_MARKER_FUNC;
	while (!stopSignal_) {
		PoolTaskHandle task(nullptr);
		std::unique_lock<std::mutex> lk(poolMutex_);
		auto pred = [this] { return stopSignal_ || !!!queuedTasks_.empty(); };
		if (!pred()) {
			PERF_MARKER_BLOCKED("idling");
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " wait for work...");
#endif
			condPendingTask_.wait(lk, pred);
		}
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " woke up.");
#endif
		if (stopSignal_)
			return;
		assertDbg(!!!queuedTasks_.empty());
		task = queuedTasks_.front();
		queuedTasks_.pop();
		lk.unlock();

		std::lock_guard<std::mutex> workLk(task->workMutex_);
		task->started_.store(true);
		// do work...
		do {
			PERF_MARKER("working");
			task->workFunc_();
		} while (0);
		task->finished_.store(true);
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " finished work.");
#endif
	}
}

void PoolTask::wait() {
	PERF_MARKER_FUNC_BLOCKED;
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " waiting for task...");
#endif
	{
		PERF_MARKER_BLOCKED("waitForTaskToStart");
		while (!started_)
			std::this_thread::yield();
	}
	PERF_MARKER_BLOCKED("waitForTaskToEnd");
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " task is started.");
#endif
	std::lock_guard<std::mutex> lk(workMutex_);
	assertDbg(finished_);
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " task is finished.");
#endif
}

void ThreadPool::checkValidState() {
	if (stopRequested_)
		throw std::runtime_error("Invalid operation on thread pool (pool is stopping)");
}
