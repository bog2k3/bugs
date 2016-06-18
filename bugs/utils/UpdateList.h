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
#include <future>

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
		// split list into 4 subsets and run them on separate threads
		uint quarterSz = std::max((size_t)1, list_.size() / 4);
		uint n1 = quarterSz;
		uint n2 = n1 == list_.size() ? n1 : n1 + quarterSz;
		uint n3 = n2 == list_.size() ? n2 : n2 + quarterSz;
		auto updateInterval = [this, dt] (uint nB, uint nE) {
			for (uint i=nB; i<nE; i++)
				list_[i].update(dt);
		};
		auto t1 = std::async(std::launch::async, updateInterval, 0, n1);
		auto t2 = std::async(std::launch::async, updateInterval, n1, n2);
		auto t3 = std::async(std::launch::async, updateInterval, n2, n3);
		auto t4 = std::async(std::launch::async, updateInterval, n3, list_.size());
		t1.wait();
		t2.wait();
		t3.wait();
		t4.wait();
	}

private:
	std::vector<updatable_wrap> list_;
	std::vector<updatable_wrap> pendingAdd_;
	std::vector<void*> pendingRemove_;
};

#endif /* UPDATELIST_H_ */
