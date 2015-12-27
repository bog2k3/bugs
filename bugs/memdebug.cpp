/*
 * memdebug.cpp
 *
 *  Created on: Dec 27, 2015
 *      Author: alexandra
 */

#include <new>
#include <set>
#include <stdexcept>

bool initialized = false;
bool internalCall = false;
std::set<void*> initialize() {
	initialized = true;
	return std::set<void*>();
}
std::set<void*> ptrSet = initialize();

void* operator new(std::size_t sz) _GLIBCXX_THROW (std::bad_alloc) {
	void* ptr = malloc(sz);
	if (initialized && !internalCall) {
		internalCall = true;
		ptrSet.insert(ptr);
		internalCall = false;
	}
	return ptr;
}

void* operator new[](std::size_t sz) _GLIBCXX_THROW (std::bad_alloc) {
	return operator new(sz);
}

void operator delete(void* ptr) _GLIBCXX_USE_NOEXCEPT {
	if (initialized && !internalCall) {
		if (ptrSet.find(ptr) == ptrSet.end())
			throw std::runtime_error("delete called on invalid pointer!!!");
		internalCall = true;
		ptrSet.erase(ptr);
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

