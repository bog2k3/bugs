/*
 * threadMarker.cpp
 *
 *  Created on: Nov 17, 2016
 *      Author: bog
 */

#include "threadMarker.h"
#include "callGraph.h"

namespace perf {

void setCrtThreadName(std::string name) {
	CallGraph::getCrtThreadInstance().threadName_ = name;
}

}


