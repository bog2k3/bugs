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

class ThreadPool {
public:
	ThreadPool(int numberOfThreads);
	~ThreadPool();	// throws exception if any thread is currently working - make sure all threads have finished prior to distruction

	void setThreadCount(unsigned count);	// throws exception if trying to lower the number while threads to be discarded are busy


	unsigned queueTask(std::function<void()> task);

	void join(unsigned id);	// wait for thread to finish its task and become available
	void detach(unsigned id);	// detach from thread and create a new one in its place
	void kill(unsigned id);	// if the thread is working, kill it and spawn a new one in its place, else do nothing

	void join_all();	// perform join() on all threads from this pool
	void detach_all();	// perform detach() on all threads from this pool
	void kill_all();	// perform kill() on all threads from this pool

protected:
	unsigned threadCount_ = 0;
	std::queue<std::function<void()>> waitingTasks_;

};



#endif /* UTILS_THREADPOOL_H_ */
