/*
 * BinaryStream.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: bogdan
 */

#include "BinaryStream.h"
#include "../utils/assert.h"
#include <cstdlib>
#include <algorithm>

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

void BinaryStream::seek(size_t offset) {
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

BinaryStream& BinaryStream::operator >> (std::string &str) {
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
	initialFilePosition_ = fileStream.tellg();
	fileStream.seekg(0, fileStream.end);
	fileSize_ = (size_t)fileStream.tellg() - initialFilePosition_;
	fileStream.seekg(initialFilePosition_);
	size_ = std::min(size_t(512), fileSize_);
	ownsBuffer_ = true;
	buffer_ = malloc(size_);
	readNextBufferChunk();
}

void BinaryStream::readNextBufferChunk() {
	if (pos_ >= fileSize_)  // this also asserts ifstream otherwise fileSize_ would be zero
		throw std::runtime_error("Attempted to read past the end of the file!");
	bool isInitialRead = (size_t)ifstream_->tellg() == initialFilePosition_;
	assert(isInitialRead || pos_ - bufferOffset_ == size_); // all internal buffer has been consumed
	if (!isInitialRead)
		bufferOffset_ += size_;
	size_t toRead = std::min(size_, fileSize_ - bufferOffset_);
	ifstream_->read((char*)buffer_, toRead);
	// check if the contents of internal buffer are less than its capacity:
	if (toRead < size_)
		size_ = toRead;
}

void BinaryStream::read(void* outBuffer, size_t size) {
	char *cbuffer = (char*)outBuffer;
	size_t readSize = 0;
	while (readSize < size) {
		// check if buffer needs update from file (if we have a file at all):
		if (pos_ - bufferOffset_ == size_)
			readNextBufferChunk();
		// copy data from the internal buffer:
		size_t positionInBuffer = pos_ - bufferOffset_;
		size_t toCopy = std::min(size-readSize, size_ - positionInBuffer);
		memcpy(cbuffer, (char*)buffer_ + positionInBuffer, toCopy);
		pos_ += toCopy;
		readSize += toCopy;
	}
}
