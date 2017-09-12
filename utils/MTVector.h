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

#include "../math/math3D.h"
#include "log.h"
#include <mutex>
#include <atomic>
#include <vector>
#include <utility>
#include <thread>

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
		src.insertPtr_.store(insertPtr_.exchange(src.insertPtr_.load(std::memory_order_consume), std::memory_order_acq_rel),
				std::memory_order_release);
		src.size_.store(size_.exchange(src.size_.load(std::memory_order_consume), std::memory_order_acq_rel),
				std::memory_order_release);
		return *this;
	}

	~MTVector() {
		clear();
		free(array_);
		array_ = nullptr;
	}

	class iterator : public std::iterator<std::random_access_iterator_tag, C> {
	public:
		C& operator *() {
			assert(pos_ < parent_.size_.load(std::memory_order_consume));
			if (extra_)
				return parent_.extra_[offs_];
			else
				return parent_.array_[offs_];
		}
		iterator& operator++() {
			move_to(pos_ + 1);
			return *this;
		}
		iterator& operator--() {
			move_to(pos_ - 1);
			return *this;
		}
		iterator& operator + (int offs) {
			move_to(pos_ + offs);
			return *this;
		}
		size_t operator - (iterator const& it) {
			return pos_ - it.pos_;
		}
		bool operator !=(iterator const& i) {
			assertDbg(&i.parent_ == &parent_ && "iterators must belong to the same vector");
			return i.pos_ != pos_;
		}
		bool operator <(iterator const& i) {
			assertDbg(&i.parent_ == &parent_ && "iterators must belong to the same vector");
			return pos_ < i.pos_;
		}
	private:
		friend class MTVector<C>;
		iterator(MTVector<C>& parent, size_t pos) : parent_(parent) {
			move_to(pos);
		}

		void move_to(size_t pos) {
			pos_ = pos;
			extra_ = pos_ >= parent_.capacity_;
			offs_ = extra_ ? pos_ - parent_.capacity_ : pos_;
		}

		MTVector<C>& parent_;
		bool extra_ = false;
		size_t pos_;
		size_t offs_;
	};

	// thread safe - block insertions from all threads and return current contents
	void getContentsExclusive(std::vector<C> &out) {
		insertionsBlocked_.store(true, std::memory_order_release);
		// wait if anyone is currently inserting:
		auto expected = insertPtr_.load(std::memory_order_relaxed);
		while (!insertPtr_.compare_exchange_weak(expected, expected,
				std::memory_order_release,
				std::memory_order_relaxed)) {
			// nothing, just loop
		}
		std::lock_guard<std::mutex> lk(extraMtx_);
		std::copy(begin(), end(), std::back_inserter(out));
		insertionsBlocked_.store(false, std::memory_order_release);
	}

	// thread safe - block insertions from all threads and return current contents
	std::vector<C> getContentsExclusive() {
		std::vector<C> ret;
		getContentsExclusive(ret);
		return ret;
	}

	// thread safe - returns index where the item was stored
	size_t push_back(C const& c) {
		return insert(c);
	}

	// thread safe - returns index where the item was stored
	size_t push_back(C &&c) {
		return insert(std::move(c));
	}

	// thread safe - returns index where the item was stored
	template<class... Args>
	size_t emplace_back(Args... args) {
		return push_back(C(args...));
	}

	// thread safe
	size_t getLockFreeCapacity() const {
		return capacity_;
	}

	// thread safe-ish (may return non-up-to-date value if another thread is writing to the vector)
	size_t size() const {
		return size_;
	}

	// thread safe-ish (may return non-up-to-date value if another thread is writing to the vector)
	bool empty() const {
		return size_ == 0;
	}

	// this is NOT thread-safe !!!
	// make sure no one is pushing data into either vector when calling this
	C& operator[] (size_t i) {
		return *iterator(*this, i);
	}

	// this is NOT thread-safe !!!
	// make sure no one is pushing data into either vector when calling this
	iterator begin() {
		return iterator(*this, 0);
	}

	// this is NOT thread-safe !!!
	// make sure no one is pushing data into either vector when calling this
	iterator end() {
		auto lastIndex = insertPtr_.load(std::memory_order_acquire);
		return iterator(*this, lastIndex >= capacity_ ? capacity_+extra_.size() : lastIndex);
	}

	C& back() {
		return *(end() - 1);
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
		insertPtr_.store(0, std::memory_order_release);
		size_.store(0, std::memory_order_release);
	}

private:
	friend class iterator;
	size_t capacity_;
	C* array_;
	std::atomic<size_t> insertPtr_ { 0 };
	std::atomic<size_t> size_ { 0 };
	std::vector<C> extra_;
	std::mutex extraMtx_;
	std::atomic<bool> insertionsBlocked_ {false};

	template<class ref>
	size_t insert(ref&& r) {
		while (insertionsBlocked_.load(std::memory_order_acquire))
			std::this_thread::yield();
		auto writeIndex = insertPtr_.fetch_add(1, std::memory_order_relaxed);
		if (writeIndex < capacity_) {
			// this is our write index
			new(array_ + writeIndex) C(std::forward<ref>(r));
		} else {
			// preallocated space filled up, must lock on the extra vector
			LOGPREFIX("MTVECTOR");
			LOGLN("PERFORMANCE WARNING: preallocated capacity reached, performing LOCK !!! capacity:" << capacity_<< "   size:" << size_.load(std::memory_order_consume));
			std::lock_guard<std::mutex> lk(extraMtx_);
			extra_.push_back(std::forward<ref>(r));
			writeIndex = extra_.size() - 1 + capacity_;
		}
		++size_;
		return writeIndex;
	}
};


#endif /* UTILS_MTVECTOR_H_ */
