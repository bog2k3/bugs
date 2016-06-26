/*
 * MTVector.h
 *
 *  Created on: Jun 26, 2016
 *      Author: bog
 */

#ifndef UTILS_MTVECTOR_H_
#define UTILS_MTVECTOR_H_

/*
 *  Multi-Threaded Vector
 *
 *  Defines a vector-like container which aims to provide thread-safe and lock-free insertions in 90% of the time
 *  When the preallocated space is exceeded, the insertion will lock, thus degrading performance (a warning will be printed)
 *
 *  1. Inserting into the vector is thread safe and lock-free as long as the preallocated space doesn't run out
 *  	(after that it will perform lock for each insertion)
 *  2. iterating over the vector is NOT thread-safe - no insertions must take place during iteration.
 *  3. clearing the vector is NOT thread-safe
 *  4. destruction is NOT thread-safe
 */

#include <mutex>
#include <atomic>
#include <vector>

template<class C>
class MTVector {
public:
	MTVector(size_t preallocatedCapacity)
		: capacity_(preallocatedCapacity)
		, array_(new  C[preallocatedCapacity])
	{
	}

	MTVector(MTVector const&) = delete;	// cannot offer lock-free, thread-safe copy constructor
	MTVector(MTVector &&) = delete;	// cannot offer lock-free, thread-safe move constructor

	~MTVector() {
		delete [] array_;
	}

	void push_back(C &c) {
		insert(c);
	}

	void push_back(C &&c) {
		insert(std::move(c));
	}

private:
	const size_t capacity_;
	C* array_;
	size_t insertPtr_ = 0;
	std::vector<C> extra_;

	template<class ref>
	void insert(ref r) {

	}
};


#endif /* UTILS_MTVECTOR_H_ */
