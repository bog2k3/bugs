/*
 * UpdateList.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: bog
 */

#include "UpdateList.h"
#include "Event.h"

UpdateList::~UpdateList() {
	// remove subscriptions from all objects in order to save them from crashing:
	for (auto pair : mapObjToRemoveSubscriptionCB) {
		pair.second();
	}
}

void UpdateList::remove(void* obj) {
	vToRemote_.push_back(obj);
}

void UpdateList::update(float dt) {
	int preSize = list_.size();
	// remove pending elements:
	for (void* obj : vToRemote_) {
		list_.erase(std::remove_if(list_.begin(), list_.end(), [obj] (const updatable_wrap& x) {
			if (x.equals(obj))
				LOGLN("found in list: " << obj);
			return x.equals(obj);
		}), list_.end());
	}
	if ((int)list_.size() != preSize)
		LOGLN("list size changed by " << (int)list_.size() - preSize);
	vToRemote_.clear();
	// do update :
	for (auto &w : list_)
		w.update(dt);
}
