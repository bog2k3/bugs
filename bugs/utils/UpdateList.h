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

class UpdateList {
public:
	UpdateList(bool allowMultithreaded = false);	// if allowMultithreaded is true, update will be done in parallel, using the thread pool
	~UpdateList() = default;

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
	bool allowMultithreaded_ = false;
};

#endif /* UPDATELIST_H_ */
