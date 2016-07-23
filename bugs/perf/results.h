/*
 * results.h
 *
 *  Created on: Jul 23, 2016
 *      Author: alexandra
 */

#ifndef PERF_RESULTS_H_
#define PERF_RESULTS_H_

#include "../utils/MTVector.h"

class Stack;

namespace perf {

class Results {
	friend class Stack;
public:

	void getResults(); // TODO implement

private:
	static MTVector<Stack*> threadStacks_;

	static void registerStack(Stack &stack) {
		threadStacks_.push_back(&stack);
	}
};

}

#endif /* PERF_RESULTS_H_ */
