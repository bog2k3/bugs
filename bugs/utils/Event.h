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

template <typename T>
class Event {
public:

	int add(std::function<T> fn) {
		callbackList_.push_back(fn);
		return callbackList_.size() - 1;
	}

	void remove(int handle) {
		assert(handle >= 0 && (unsigned)handle < callbackList_.size());
		callbackList_[handle] = nullptr;
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
