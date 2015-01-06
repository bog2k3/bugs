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

class UpdateList {
public:
	UpdateList() {}
	~UpdateList() {}

	void add(updatable_wrap w) {
		list_.push_back(w);
	}

	void remove(updatable_wrap w) {
		list_.erase(std::remove_if(list_.begin(), list_.end(), [&w] (const updatable_wrap& x) {
			return x.equal_value(w);
		}), list_.end());
	}

	void update(float dt) {
		for (auto &w : list_)
			w.update(dt);
	}

private:
	std::vector<updatable_wrap> list_;
};

template <> void update(UpdateList* &l, float dt);

#endif /* UPDATELIST_H_ */
