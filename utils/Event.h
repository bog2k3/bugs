/*
 * Event.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <functional>
#include <vector>
#include <cassert>
#include "assert.h"

template <typename T>
class Event {
public:

	Event() = default;

	Event(Event &&e)
		: callbackList_(std::move(e.callbackList_)) {
	}

	int add(std::function<T> fn) {
		callbackList_.push_back(fn);
		return callbackList_.size() - 1;
	}

	void remove(int handle) {
		assertDbg(handle >= 0 && (unsigned)handle < callbackList_.size());
		callbackList_[handle] = nullptr;
	}

	void clear() {
		callbackList_.clear();
	}

	void trigger() {
		for (auto c : callbackList_)
			c();
	}

	template<typename... argTypes>
	void trigger(argTypes... argList) {
		for (auto c : callbackList_)
			if (c)
				c(argList...);
	}

protected:
	std::vector<std::function<T>> callbackList_;
};

#endif /* EVENT_H_ */
