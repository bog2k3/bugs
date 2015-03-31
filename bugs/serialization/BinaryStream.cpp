/*
 * BinaryStream.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: bogdan
 */

#include "BinaryStream.h"
#include "../utils/assert.h"
#include <cstdlib>

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

BinaryStream& BinaryStream::operator << (std::string const& str) {
	operator <<((uint16_t)str.length());
	for (int i=0, n=str.length(); i<n; i++)
		operator <<((uint16_t)str[i]);
	return *this;
}

const BinaryStream& BinaryStream::operator >> (std::string &str) const {
	uint16_t length = 0;
	operator >>(length);
	str = "";
	for (int i=0; i<length; i++) {
		uint16_t c = 0;
		operator >>(c);
		str += c;
	}
	return *this;
}

BinaryStream::BinaryStream(std::ifstream &fileStream)
	: ifstream_(&fileStream)
{
	if (!fileStream.is_open())
		throw std::runtime_error("BinaryStream constructed over closed std::ifstream");
}

void BinaryStream::read(void* buffer, size_t size) {
#error "must fix all >> operators to work correctly on ifstream; use buffer_ as temp buffer to read from ifstream"
	// also fix all seek, getpos and stuff for this case
}
