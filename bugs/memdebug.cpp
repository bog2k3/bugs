/*
 * memdebug.cpp
 *
 *  Created on: Dec 27, 2015
 *      Author: bogdan
 */

#include <new>
#include <set>
#include <stdexcept>
#include <mutex>

//#define ENABLE_MEMDEBUG

#ifdef ENABLE_MEMDEBUG

bool initialized = false;
bool internalCall = false;
std::set<void*> initialize() {
	initialized = true;
	return std::set<void*>();
}

std::recursive_mutex heapMutex;

static struct wrapper {
	std::set<void*> ptrSet = initialize();

	~wrapper() {
		std::lock_guard<std::recursive_mutex> lk(heapMutex);
		internalCall = true;
		ptrSet.clear();
		internalCall = false;
		initialized = false;
	}
} _;


void* operator new(std::size_t sz) _GLIBCXX_THROW (std::bad_alloc) {
	std::lock_guard<std::recursive_mutex> lk(heapMutex);
	void* ptr = malloc(sz);
	if (initialized && !internalCall) {
		internalCall = true;
		_.ptrSet.insert(ptr);
		internalCall = false;
	}
	return ptr;
}

void* operator new[](std::size_t sz) _GLIBCXX_THROW (std::bad_alloc) {
	return operator new(sz);
}

void operator delete(void* ptr) _GLIBCXX_USE_NOEXCEPT {
	std::lock_guard<std::recursive_mutex> lk(heapMutex);
	if (initialized && !internalCall) {
		if (_.ptrSet.find(ptr) == _.ptrSet.end())
			throw std::runtime_error("delete called on invalid pointer!!!");
		internalCall = true;
		_.ptrSet.erase(ptr);
		internalCall = false;
	}
	free(ptr);
}

void operator delete[](void* ptr) _GLIBCXX_USE_NOEXCEPT {
	operator delete(ptr);
}

void* operator new(std::size_t sz, const std::nothrow_t&) _GLIBCXX_USE_NOEXCEPT {
	return operator new(sz);
}

void* operator new[](std::size_t sz, const std::nothrow_t&) _GLIBCXX_USE_NOEXCEPT {
	return operator new[](sz);
}

void operator delete(void* ptr, const std::nothrow_t&) _GLIBCXX_USE_NOEXCEPT {
	operator delete(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) _GLIBCXX_USE_NOEXCEPT {
	operator delete[](ptr);
}

#endif
