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
#include <algorithm>
#include <iterator>

class UpdateList {
public:
	UpdateList() = default;

	void add(updatable_wrap &&w) {
		pendingAdd_.push_back(w);
	}

	void remove(void* ptr) {
		pendingRemove_.push_back(ptr);
	}

	void update(float dt) {
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
		for (auto &w : list_)
			w.update(dt);
	}

private:
	std::vector<updatable_wrap> list_;
	std::vector<updatable_wrap> pendingAdd_;
	std::vector<void*> pendingRemove_;
};

#endif /* UPDATELIST_H_ */
