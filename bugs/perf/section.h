/*
 * section.h
 *
 *  Created on: Jul 22, 2016
 *      Author: bog
 */

#ifndef PERF_SECTION_H_
#define PERF_SECTION_H_

#include <unordered_map>
#include <memory>

namespace perf {

struct charArrHash
{
	size_t operator()(const char s[]) const
	{
		size_t h = 5381;
		int c;
		const char* s0 = s;
		while ((c = *s0++))
			h = ((h << 5) + h) + c;
		return h;
	}
};

class Section {
	friend class Stack;
private:
	Section(const char name[]) {
		strncpy(name_, name, sizeof(name_)/sizeof(name_[0]));
	}

	unsigned nanoseconds_ = 0;
	unsigned callCount_ = 0;
	char name_[128];

	std::unordered_map<char[], std::shared_ptr<Section>, charArrHash> subsections_;
};

}

#endif /* PERF_SECTION_H_ */
