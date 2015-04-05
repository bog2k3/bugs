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

static union {
	uint32_t i;
	char c[4];
} constexpr bigEndianTest {0x01020304};

#define IS_BIG_ENDIAN (bigEndianTest.c[0] == 1)
#define IS_LITTLE_ENDIAN (!(IS_BIG_ENDIAN))

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

/**
 * inputs data into the stream, incrementing the position. If the size of the stream grows larger than its capacity:
 * 	a. the capacity is increased if the stream owns the underlying buffer
 * 	OR:
 * 	b. an exception is generated if the underlying buffer is not owned by the stream.
 */
template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
BinaryStream& BinaryStream::operator << (T const& t) {
	size_t dataSize = sizeof(t);
	if (pos_ + dataSize > capacity_) {
		if (ownsBuffer_)
			expandBuffer();
		else
			throw std::runtime_error("attempted to write past the end of unmanaged buffer!");
	}
	if (IS_LITTLE_ENDIAN) {
		// little endian, write directly:
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
	return *this;
}


/**
 * outputs data from the stream, incrementing the position.
 * If attempting to read past the end of the stream, an exception is generated.
 */
template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type*>
BinaryStream& BinaryStream::operator >> (T &t) {
	size_t dataSize = sizeof(t);
	size_t maxSize = ifstream_ ? fileSize_ : size_;
	if (pos_ + dataSize > maxSize)
		throw std::runtime_error("attempted to read past the end of the buffer!");
	void *readBuffer = buffer_;
	size_t *readPos = &pos_;
	// if we're over an ifstream, must use a temp buffer into which we copy the data, then we deserialize from it
	// because the main buffer may not contain all the bytes needed, and a re-read from file may occur half-way
	char tempBuffer[32];
	size_t tempReadPos = 0;
	assertDbg(dataSize < sizeof(tempBuffer)); // if this ever fails, we need to increase tempBuffer capacity
	if (ifstream_) {
		readBuffer = tempBuffer;
		readPos = &tempReadPos;
		read((void*)tempBuffer, dataSize);
	}
	if (IS_LITTLE_ENDIAN) {
		// little endian, read directly:
		memcpy(&t, (char*)readBuffer+(*readPos), dataSize);
		*readPos += dataSize;
	} else {
		// big endian, must write byte by byte in reverse order
		char* ptr = (char*)&t + dataSize-1;
		while (ptr >= (char*)&t) {
			*(ptr--) = *(((char*)readBuffer) + (*readPos)++);
		}
	}
	return *this;
}

BinaryStream& BinaryStream::operator << (int8_t const& i) {
	return operator<< <int8_t>(i);
}

BinaryStream& BinaryStream::operator << (uint8_t const& i) {
	return operator<< <uint8_t>(i);
}

BinaryStream& BinaryStream::operator << (int16_t const& i) {
	return operator<< <int16_t>(i);
}

BinaryStream& BinaryStream::operator << (uint16_t const& i) {
	return operator<< <uint16_t>(i);
}

BinaryStream& BinaryStream::operator << (int32_t const& i) {
	return operator<< <int32_t>(i);
}

BinaryStream& BinaryStream::operator << (uint32_t const& i) {
	return operator<< <uint32_t>(i);
}

BinaryStream& BinaryStream::operator << (int64_t const& i) {
	return operator<< <int64_t>(i);
}

BinaryStream& BinaryStream::operator << (uint64_t const& i) {
	return operator<< <uint64_t>(i);
}

BinaryStream& BinaryStream::operator << (float const& f) {
	static_assert(sizeof(float) == 4, "if this ever fails, must find a 4-byte float type to convert to before serializing");
	return operator<< <float>(f);
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
	str = ""; str.reserve(length);
	for (int i=0; i<length; i++) {
		uint16_t c = 0;
		operator >>(c);
		str += c;
	}
	return *this;
}

BinaryStream& BinaryStream::operator >> (int8_t &i) {
	return operator>> <int8_t>(i);
}

BinaryStream& BinaryStream::operator >> (uint8_t &i) {
	return operator>> <uint8_t>(i);
}

BinaryStream& BinaryStream::operator >> (int16_t &i) {
	return operator>> <int16_t>(i);
}

BinaryStream& BinaryStream::operator >> (uint16_t &i) {
	return operator>> <uint16_t>(i);
}

BinaryStream& BinaryStream::operator >> (int32_t &i) {
	return operator>> <int32_t>(i);
}

BinaryStream& BinaryStream::operator >> (uint32_t &i) {
	return operator>> <uint32_t>(i);
}

BinaryStream& BinaryStream::operator >> (int64_t &i) {
	return operator>> <int64_t>(i);
}

BinaryStream& BinaryStream::operator >> (uint64_t &i) {
	return operator>> <uint64_t>(i);
}

BinaryStream& BinaryStream::operator >> (float &f) {
	static_assert(sizeof(float) == 4, "if this ever fails, must find a 4-byte float type to convert to before serializing");
	return operator>> <float>(f);
}
