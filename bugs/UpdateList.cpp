/*
 * UpdateList.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: bog
 */

#include "UpdateList.h"
#include "Event.h"

void UpdateList::add(updatable_wrap &&w) {
	list_.push_back(w);
}

void UpdateList::update(float dt) {
	int validSize = list_.size();
	// do update :
	for (int i=0; i<validSize; i++) {
		if (list_[i].isAlive())
			list_[i].update(dt);
		else
			list_[i] = list_[--validSize];
	}
	list_.erase(list_.begin()+validSize, list_.end());
}
