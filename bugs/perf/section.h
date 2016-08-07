/*
 * section.h
 *
 *  Created on: Jul 22, 2016
 *      Author: bog
 */

#ifndef PERF_SECTION_H_
#define PERF_SECTION_H_

#include <cstring>

namespace perf {

class sectionData {
	friend class CallGraph;
private:
	sectionData(const char name[]) {
		strncpy(name_, name, sizeof(name_)/sizeof(name_[0]));
	}

	unsigned nanoseconds_ = 0;
	unsigned executionCount_ = 0;
	char name_[256];
};

}

#endif /* PERF_SECTION_H_ */
