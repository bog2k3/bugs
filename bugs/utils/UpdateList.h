/*
 * UpdateList.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef UPDATELIST_H_
#define UPDATELIST_H_

#include "updatable.h"
#include <vector>

class ThreadPool;

class UpdateList {
public:
	UpdateList(uint parallelThreads = 1);
	~UpdateList();

	void add(updatable_wrap &&w) {
		pendingAdd_.push_back(w);
	}

	void remove(void* ptr) {
		pendingRemove_.push_back(ptr);
	}

	void update(float dt);

private:
	std::vector<updatable_wrap> list_;
	std::vector<updatable_wrap> pendingAdd_;
	std::vector<void*> pendingRemove_;
	uint parallelThreadCount_;
	ThreadPool* pThreadPool = nullptr;
};

#endif /* UPDATELIST_H_ */
