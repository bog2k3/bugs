/*
 * stack.cpp
 *
 *  Created on: Jul 23, 2016
 *      Author: alexandra
 */

#include "stack.h"
#include "results.h"

namespace perf {

thread_local static Stack Stack::crtInstance_;

Stack::Stack() {
	Results::registerStack(*this);
	stack_.push(std::make_shared<Section>("_")); // root node
}

}
