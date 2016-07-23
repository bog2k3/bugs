/*
 * stack.h
 *
 *  Created on: Jul 23, 2016
 *      Author: alexandra
 */

#ifndef PERF_STACK_H_
#define PERF_STACK_H_

#include "section.h"

namespace perf {

class Stack {
	friend class Marker;
private:
	static void push(const char sectionName[]) {
		auto it = stack_.front()->subsections_.find(sectionName);
	}

	static void pop(unsigned nanoseconds) {

	}

	Stack();

	std::stack<std::shared_ptr<Section>> stack_;

	thread_local static Stack crtInstance_;
};

}

#endif /* PERF_STACK_H_ */
