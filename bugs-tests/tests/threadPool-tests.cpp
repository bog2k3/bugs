/*
 * threadPool-tests.cpp
 *
 *  Created on: Jun 22, 2016
 *      Author: bog
 */

#include "../../bugs/utils/ThreadPool.h"

#include <array>
#include <iostream>

#include <easyunit/test.h>
using namespace easyunit;

bool isPrime(int x) {
	for (int y=2; y<x; y++)
		if ((x%y) == 0)
			return false;
	return true;
}

TEST(threadPool, parallelFor) {
	const uint nThreads = 4;
	ThreadPool pool(nThreads);
	std::vector<int> x;
	uint workSize = 100000;
	for (uint i=0; i<workSize; i++)
		x.push_back(i);
	auto func = [&x] (uint a, uint b) {
		for (uint i=a; i<b; i++) {
			// work on ith element
			x[i] = isPrime(x[i]) ? x[i] : 0;
		}
	};
	uint chunkSize = workSize / nThreads;
	uint start = 0;
	std::array<PoolTaskHandle, nThreads> tasks;
	for (uint i=0; i<nThreads; i++) {
		tasks[i] = pool.queueTask(func, start, start+chunkSize);
		start += chunkSize;
	}
	for (auto &t : tasks)
		t->wait();
	pool.stop();

	for (uint i=0; i<workSize; i++) {
		int expected = isPrime(x[i]) ? x[i] : 0;
		ASSERT_EQUALS(expected, x[i]);
	}
}
