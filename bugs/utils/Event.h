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
		assert(handle >= 0 && handle < callbackList_.size());
		callbackList_[handle] = nullptr;
	}

	void trigger() {
		for (auto c : callbackList_)
			c();
	}

	template<typename T1>
	void trigger(T1 const &t1) {
		for (auto c : callbackList_)
			if (c)
				c(t1);
	}

	template<typename T1>
	void trigger(T1 &t1) {
		for (auto c : callbackList_)
			if (c)
				c(t1);
	}

	template<typename T1>
	void trigger(T1 const &t1) const {
		for (auto c : callbackList_)
			c(t1);
	}

	template<typename T1, typename T2>
	void trigger(T1 const &t1, T2 const &t2) {
		for (auto c : callbackList_)
			c(t1, t2);
	}

	template<typename T1, typename T2>
	void trigger(T1 const &t1, T2 const &t2) const {
		for (auto c : callbackList_)
			c(t1, t2);
	}

	template<typename T1, typename T2, typename T3>
	void trigger(T1 const &t1, T2 const &t2, T3 const &t3) {
		for (auto c : callbackList_)
			c(t1, t2, t3);
	}

	template<typename T1, typename T2, typename T3>
	void trigger(T1 const &t1, T2 const &t2, T3 const &t3) const {
		for (auto c : callbackList_)
			c(t1, t2, t3);
	}

protected:
	std::vector<std::function<T>> callbackList_;
};

#endif /* EVENT_H_ */
