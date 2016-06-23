/*
 * Infrastructure.h
 *
 *  Created on: Jun 23, 2016
 *      Author: bog
 */

#ifndef INFRASTRUCTURE_H_
#define INFRASTRUCTURE_H_

#include "utils/ThreadPool.h"

class Infrastructure {
public:
	static Infrastructure& getInst() {
		static Infrastructure instance;
		return instance;
	}

private:
	Infrastructure() = default;
};


#endif /* INFRASTRUCTURE_H_ */
