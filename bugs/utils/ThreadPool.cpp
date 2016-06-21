/*
 * ThreadPool.cpp
 *
 *  Created on: Jun 21, 2016
 *      Author: bog
 */

#include "ThreadPool.h"

ThreadPool::ThreadPool(int numberOfThreads)
	: threadCount_(numberOfThreads)
{
	for (int i=0; i<numberOfThreads; i++)
		workers_.push_back(spawnThread());
}
ThreadPool::~ThreadPool(){	// throws exception if any thread is currently working - make sure all threads have finished prior to distruction
	// TODO
	throw std::runtime_error("Not implemented");
}

ThreadPool::threadWrap ThreadPool::spawnThread() {
	return threadWrap(*this);
}

void ThreadPool::setThreadCount(unsigned count) {	// throws exception if trying to lower the number while threads to be discarded are busy
	// TODO
	throw std::runtime_error("Not implemented");
}

void ThreadPool::wait(unsigned id) {
	// wait for thread to finish its task and become available
	if (id > workers_.size())
		throw std::runtime_error("invalid worker ID");
	workers_[id].wait();
}

void ThreadPool::detach(unsigned id) {
	// detach from thread and create a new one in its place
	if (id > workers_.size())
		throw std::runtime_error("invalid worker ID");
	workers_[id].detach();
}

void ThreadPool::kill(unsigned id) {
	// if the thread is working, kill it and spawn a new one in its place, else do nothing
	if (id > workers_.size())
		throw std::runtime_error("invalid worker ID");
	workers_[id].kill();
}

void ThreadPool::wait_all() {
	// perform join() on all threads from this pool
	for (auto &w : workers_)
		w.wait();
}

void ThreadPool::detach_all() {
	// perform detach() on all threads from this pool
	for (auto &w : workers_)
		w.detach();
}

void ThreadPool::kill_all() {
	// perform kill() on all threads from this pool
	for (auto &w : workers_)
		w.kill();
}

void ThreadPool::threadWrap::workerFunc() {
	//while (!own)
}
