/*
 * UpdateList.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: bog
 */

#include "UpdateList.h"
#include "Event.h"

template <> void update(UpdateList* l, float dt) {
	l->update(dt);
}

void UpdateList::add(updatable_wrap &&w) {
	w.onDestroy.add(std::bind(&UpdateList::remove, this, w));
	list_.push_back(std::move(w));
}

void UpdateList::remove(updatable_wrap w) {
	list_.erase(std::remove_if(list_.begin(), list_.end(), [&w] (const updatable_wrap& x) {
		return x.equal_value(w);
	}), list_.end());
}

void UpdateList::update(float dt) {
	for (auto &w : list_)
		w.update(dt);
}
