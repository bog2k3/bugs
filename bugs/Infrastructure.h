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
	// call this before exiting in order to stop the thread pool and free resources
	void shutDown();

	ThreadPool& getThreadPool() { return threadPool_; }

private:
	Infrastructure();

	ThreadPool threadPool_;
};


#endif /* INFRASTRUCTURE_H_ */
