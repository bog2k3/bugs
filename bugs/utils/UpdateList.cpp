/*
 * UpdateList.cpp
 *
 *  Created on: Jun 18, 2016
 *      Author: bog
 */

#include "UpdateList.h"

void UpdateList::update(float dt) {
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
	// TODO: split list into 4 subsets and run them on separate threads
	// use a thread pool for this
	uint quarterSz = std::max((size_t)1, list_.size() / 4);
	if (list_.size() == 0)
		quarterSz = 0;
	uint n1 = quarterSz;
	uint n2 = n1 == list_.size() ? n1 : n1 + quarterSz;
	uint n3 = n2 == list_.size() ? n2 : n2 + quarterSz;
	auto updateInterval = [this, dt] (uint nB, uint nE) {
		for (uint i=nB; i<nE; i++)
			list_[i].update(dt);
	};
	updateInterval(0, n1);
	updateInterval(n1, n2);
	updateInterval(n2, n3);
	updateInterval(n3, list_.size());
}

