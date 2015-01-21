/*
 * UpdateList.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef UPDATELIST_H_
#define UPDATELIST_H_

#include "updatable.h"
#include "log.h"
#include <vector>
#include <algorithm>
#include <map>

class UpdateList {
public:
	UpdateList() {}
	~UpdateList();

	template<typename T>
	void add(T* t) {
		updatable_wrap w(t);
		list_.push_back(w);
		subscribe(t);
	}

	void remove(void* obj_);
	void update(float dt);

private:
	template<typename T>
	decltype(T::onDestroy)* subscribe(T* t) {
		int subscriptionId = t->onDestroy.add([this](T* t) {
			LOGLN("remove " << t);
			remove(t);
			mapObjToRemoveSubscriptionCB.erase(mapObjToRemoveSubscriptionCB.find(t));
		});
		auto removeSubscriptionCB = [t, subscriptionId] {
			t->onDestroy.remove(subscriptionId);
		};
		mapObjToRemoveSubscriptionCB[t] = removeSubscriptionCB;
		return nullptr;
	}
	void subscribe(...) {
	}

	std::vector<updatable_wrap> list_;
	std::vector<void*> vToRemote_;
	std::map<void*, std::function<void()>> mapObjToRemoveSubscriptionCB;
};

#endif /* UPDATELIST_H_ */
