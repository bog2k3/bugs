/*
 * BinaryStream.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: bogdan
 */

#include "BinaryStream.h"
#include "../utils/assert.h"
#include <cstdlib>
#include <memory.h>
#include <stdexcept>

static constexpr union {
	uint32_t i;
	char c[4];
} bigEndianTest = {0x01020304};

constexpr bool is_big_endian()
{
	return bigEndianTest.c[0] == 1;
}

BinaryStream::BinaryStream(size_t initial_capacity) {
	capacity_ = initial_capacity;
	buffer_ = malloc(capacity_);
	ownsBuffer_ = true;
}
BinaryStream::BinaryStream(void* buffer, size_t size) {
	buffer_ = buffer;
	capacity_ = size_ = size;
}

BinaryStream::~BinaryStream() {
	if (ownsBuffer_) {
		free(buffer_);
		buffer_ = nullptr;
	}
}

void BinaryStream::seek(size_t offset) const {
	assertDbg(offset <= size_);
	pos_ = offset;
}

void BinaryStream::expandBuffer() {
	assertDbg(ownsBuffer_);
	void* newBuf = malloc(capacity_*=2);
	memcpy(newBuf, buffer_, size_);
	free(buffer_);
	buffer_ = newBuf;
}

template<typename T, typename std::enable_if<std::is_fundamental<T>::value>::type>
BinaryStream& BinaryStream::operator << (T& t) {
	size_t dataSize = sizeof(t);
	if (pos_ + dataSize > capacity_) {
		if (ownsBuffer_)
			expandBuffer();
		else
			throw std::runtime_error("attempted to write past the end of unmanaged buffer!");
	}
	if (!!!is_big_endian()) {
		// small endian, write directly:
		memcpy((char*)buffer_+pos_, &t, dataSize);
		pos_ += dataSize;
		if (pos_ > size_)
			size_ = pos_;
	} else {
		// big endian, must write byte by byte in reverse order
		char* ptr = (char*)&t + dataSize-1;
		while (ptr >= (char*)&t) {
			*(((char*)buffer_) + pos_++) = *(ptr--);
		}
		if (pos_ > size_)
			size_ = pos_;
	}
}

template<typename T, typename std::enable_if<std::is_fundamental<T>::value>::type>
BinaryStream& BinaryStream::operator >> (T& t) const {
	size_t dataSize = sizeof(t);
	if (pos_ + dataSize > size_)
		throw std::runtime_error("attempted to read past the end of the buffer!");
	if (!!!is_big_endian()) {
		// small endian, read directly:
		memcpy(&t, (char*)buffer_+pos_, dataSize);
		pos_ += dataSize;
	} else {
		// big endian, must write byte by byte in reverse order
		char* ptr = (char*)&t + dataSize-1;
		while (ptr >= (char*)&t) {
			*(ptr--) = *(((char*)buffer_) + pos_++);
		}
	}
}
