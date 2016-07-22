/*
 * section.h
 *
 *  Created on: Jul 22, 2016
 *      Author: bog
 */

#ifndef PERF_SECTION_H_
#define PERF_SECTION_H_

namespace perf {

class sectionData {
private:
	friend class Marker;
	sectionData(const char name[], unsigned nanosec);
};

}

#endif /* PERF_SECTION_H_ */
