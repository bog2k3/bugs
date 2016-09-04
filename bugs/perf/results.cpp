/*
 * results.cpp
 *
 *  Created on: Jul 23, 2016
 *      Author: alexandra
 */

#include "results.h"

namespace perf {

MTVector<std::shared_ptr<CallGraph>> Results::threadGraphs_ { 16 };

}
