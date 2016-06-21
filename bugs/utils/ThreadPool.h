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

class ThreadPool {
public:
	ThreadPool(int numberOfThreads);
	~ThreadPool();	// throws exception if any thread is currently working - make sure all threads have finished prior to distruction

	template<class F>
	void queueTask(F task) {
		std::lock_guard<std::mutex> lk(poolMutex_);
		waitingTasks_.push([=] { task(); });
		condPendingTask_.notify_one();
	}

	void wait(unsigned id);	// blocks until thread finishes its task and becomes available
	void detach(unsigned id);	// detach from thread and create a new one in its place
	void kill(unsigned id);	// if the thread is working, kill it and spawn a new one in its place, else do nothing

	void wait_all();	// perform wait() on all threads from this pool
	void detach_all();	// perform detach() on all threads from this pool
	void kill_all();	// perform kill() on all threads from this pool

protected:
	friend struct threadWrap;
	struct threadWrap {
		enum THREAD_STATE {
			INITIALIZING,
			READY,
			WORKING,
			STOPPING,
			DEAD,
		};
		ThreadPool &owner_;
		std::thread thread_;
		std::atomic<THREAD_STATE> state_ { INITIALIZING };
		std::mutex workMutex_;	// locked while doing work

		threadWrap(ThreadPool &owner)
			: owner_(owner)
			, thread_(std::bind(&threadWrap::workerFunc, this))
		{
		}

		threadWrap(threadWrap &&t)
			: owner_(t.owner_)
			, thread_(std::move(t.thread_))
			, state_(t.state_.load()) {
		}

		void wait();	// blocks until the worker finishes the current task
		void detach();	// detaches the underlying thread and spawn a new one
		void kill();	// kills the underlying thread (if working) and spawns a new one

	private:
		void workerFunc();
	};

	struct taskWrap {
		std::function<void()> func_;
		std::atomic<bool> finished_ { false };
	};

	unsigned threadCount_ = 0;
	std::queue<> waitingTasks_;
	std::mutex poolMutex_;
	std::condition_variable condPendingTask_;
	std::vector<threadWrap> workers_;
	std::atomic<bool> stop_ { false };

	threadWrap spawnThread();
};



#endif /* UTILS_THREADPOOL_H_ */
