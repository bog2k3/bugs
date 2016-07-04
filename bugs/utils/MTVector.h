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

#include "log.h"
#include "../math/math2D.h"
#include <mutex>
#include <atomic>
#include <vector>
#include <utility>

template<class C>
class MTVector {
public:
	MTVector(size_t preallocatedCapacity)
		: capacity_(preallocatedCapacity)
		, array_(static_cast<C*>(malloc(sizeof(C)*preallocatedCapacity)))
	{
	}

	// this is NOT thread-safe !!!
	// make sure no one is accessing the source object while calling this
	MTVector(MTVector const& src)
		: capacity_(src.capacity_)
		, array_(static_cast<C*>(malloc(sizeof(C)*capacity_)))
		, insertPtr_(src.insertPtr_.load())
		, extra_(src.extra_)
	{
		for (size_t i=0; i<insertPtr_; i++) {
			new (array_+i) C(src.array_[i]);
		}
	}

	// this is NOT thread-safe !!!
	// make sure no one is accessing the source object while calling this
	MTVector(MTVector &&src)
		: capacity_(0)
		, array_(nullptr)
	{
		operator =(std::move(src));
	}

	// this is NOT thread-safe !!!
	// make sure no one is accessing the source object while calling this
	MTVector& operator = (MTVector&& src) {
		xchg(array_, src.array_);
		xchg(capacity_, src.capacity_);
		extra_.swap(src.extra_);
		insertPtr_.exchange(src.insertPtr_, std::memory_order_acq_rel);
		return *this;
	}

	~MTVector() {
		clear();
		free(array_);
	}

	class iterator {
	public:
		C& operator *() {
			if (extra_)
				return parent_.extra_[offs_];
			else
				return parent_.array_[offs_];
		}
		iterator& operator++() {
			move_to(pos_+1);
			return *this;
		}
		bool operator !=(iterator &i) {
			assertDbg(&i.parent_ == &parent_ && "iterators must belong to the same vector");
			return i.pos_ != pos_;
		}
	private:
		friend class MTVector<C>;
		iterator(MTVector<C> &parent, size_t pos) : parent_(parent) {
			move_to(pos);
		}

		void move_to(size_t pos) {
			pos_ = pos;
			extra_ = pos_ >= parent_.capacity_;
			offs_ = extra_ ? pos_ - parent_.capacity_ : pos_;
		}

		MTVector<C> &parent_;
		bool extra_ = false;
		size_t pos_;
		size_t offs_;
	};

	// thread safe
	void push_back(C const& c) {
		insert(c);
	}

	// thread safe
	void push_back(C &&c) {
		insert(std::move(c));
	}

	// thread safe
	size_t getLockFreeCapacity() const {
		return capacity_;
	}

	// this is NOT thread-safe !!!
	// make sure no one is pushing data into either vector when calling this
	iterator begin() {
		return iterator(*this, 0);
	}

	// this is NOT thread-safe !!!
	// make sure no one is pushing data into either vector when calling this
	iterator end() {
		return iterator(*this, insertPtr_ >= capacity_ ? capacity_+extra_.size() : insertPtr_.load());
	}

	// this is NOT thread-safe !!!
	// make sure no one is pushing data into either vector when calling this
	void swap(MTVector<C> &other) {
		operator =(std::move(other));
	}

	// this is NOT thread-safe !!!
	// make sure no one is pushing data into either vector when calling this
	void clear() {
		extra_.clear();
		for (size_t i=0; i<min(capacity_, insertPtr_.load()); i++)
			array_[i].~C();
		insertPtr_.store(0);
	}

private:
	friend class iterator;
	size_t capacity_;
	C* array_;
	std::atomic<size_t> insertPtr_ { 0 };
	std::vector<C> extra_;
	std::mutex extraMtx_;

	template<class ref>
	void insert(ref&& r) {
		auto expected = insertPtr_.load(std::memory_order_relaxed);
		while (!insertPtr_.compare_exchange_weak(expected, expected+1,
				std::memory_order_release,
				std::memory_order_relaxed)) {
			// nothing, just loop
		}
		if (expected < capacity_) {
			// this is our write index
			new(array_ + expected) C(std::forward<ref>(r));
		} else {
			// preallocated space filled up, must lock on the extra vector
			LOGPREFIX("MTVECTOR");
			LOG("PERFORMANCE WARNING: preallocated capacity reached, performing LOCK !!!");
			std::lock_guard<std::mutex> lk(extraMtx_);
			extra_.push_back(std::forward<ref>(r));
		}
	}
};


#endif /* UTILS_MTVECTOR_H_ */
