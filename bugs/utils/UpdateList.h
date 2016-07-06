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
	void add(updatable_wrap &&w) {
		pendingAdd_.push_back(w);
	}

	void remove(void* ptr) {
		pendingRemove_.push_back(ptr);
	}

	void update(float dt, ThreadPool *pool = nullptr);

private:
	std::vector<updatable_wrap> list_;
	std::vector<updatable_wrap> pendingAdd_;
	std::vector<void*> pendingRemove_;
};

#endif /* UPDATELIST_H_ */
