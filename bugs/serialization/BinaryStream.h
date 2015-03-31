/*
 * BinaryStream.h
 *
 *  Created on: Mar 24, 2015
 *      Author: bogdan
 */

#ifndef SERIALIZATION_BINARYSTREAM_H_
#define SERIALIZATION_BINARYSTREAM_H_

#include "../utils/assert.h"
#include <type_traits>
#include <cstddef>
#include <string>
#include <stdexcept>
#include <memory.h>
#include <fstream>

static union {
	uint32_t i;
	char c[4];
} constexpr bigEndianTest {0x01020304};

#define IS_BIG_ENDIAN (bigEndianTest.c[0] == 1)
#define IS_LITTLE_ENDIAN (!(IS_BIG_ENDIAN))

class BinaryStream {
public:
	/**
	 * creates an empty stream with initial capacity. The stream's buffer is managed by this instance.
	 * This is ready to be written to.
	 */
	BinaryStream(size_t initial_capacity);
	/**
	 * creates a read/write stream over an existing buffer.
	 * The underlying buffer is owned by the caller and cannot be resized.
	 */
	BinaryStream(void* buffer, size_t size);
	/**
	 * creates a READ-ONLY binary stream over the specified file. The file must be opened in binary mode otherwise
	 * an std::runtime_error is thrown.
	 * The BinaryStream will read data from the file as needed.
	 */
	BinaryStream(std::ifstream &fileStream);
	#warning "must fix  all seek, getpos and stuff for this case"

	virtual ~BinaryStream();

	size_t getCapacity() const { return capacity_; }
	size_t getSize() const { return ifstream_ ? fileSize_ : size_; }
	const void* getBuffer() const { assertDbg(!ifstream_); return buffer_; }
	void seek(size_t offset);
	bool eof() { return ifstream_ ? pos_ >= fileSize_ : pos_ >= size_; }

	/**
	 * reads raw data from the stream and copies it into the supplied buffer.
	 * The data will be written exactly as encoded in the stream, no translations or deserialization will occur.
	 */
	void read(void* buffer, size_t size);

	/**
	 * inputs data into the stream, incrementing the position. If the size of the stream grows larger than its capacity:
	 * 	a. the capacity is increased if the stream owns the underlying buffer
	 * 	OR:
	 * 	b. an exception is generated if the underlying buffer is not owned by the stream.
	 */
	template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
	BinaryStream& operator << (T const& t) {
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

	BinaryStream& operator << (const std::string &str);

	/**
	 * outputs data from the stream, incrementing the position.
	 * If attempting to read past the end of the stream, an exception is generated.
	 */
	template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
	BinaryStream& operator >> (T &t) {
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

	BinaryStream& operator >> (std::string &str);

protected:
	size_t capacity_ = 0;
	size_t size_ = 0;
	size_t pos_ = 0;
	size_t bufferOffset_ = 0; // for BinaryStreams over ifstream; tells where in the file our internal buffer begins
	size_t fileSize_ = 0; // for BinaryStreams over ifstream;
	size_t initialFilePosition_ = 0;
	void *buffer_ = nullptr;
	bool ownsBuffer_ = false;
	std::ifstream *ifstream_ = nullptr;

	void expandBuffer(); // for write-BinaryStream: expands the internal buffer by doubling it's size
	void readNextBufferChunk(); // (for BinaryStream over ifstream only) reads next chunk from the file into the internal buffer
};

#endif /* SERIALIZATION_BINARYSTREAM_H_ */
