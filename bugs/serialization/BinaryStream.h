/*
 * BinaryStream.h
 *
 *  Created on: Mar 24, 2015
 *      Author: bogdan
 */

#ifndef SERIALIZATION_BINARYSTREAM_H_
#define SERIALIZATION_BINARYSTREAM_H_

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

	virtual ~BinaryStream();

	size_t getCapacity() const { return capacity_; }
	size_t getSize() const { return size_; }
	const void* getBuffer() const { return buffer_; }
	void seek(size_t offset) const;
	bool eof() { return pos_ >= size_; }

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
	BinaryStream& operator << (T t) {
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
	const BinaryStream& operator >> (T t) const {
		size_t dataSize = sizeof(t);
		if (pos_ + dataSize > size_)
			throw std::runtime_error("attempted to read past the end of the buffer!");
		if (IS_LITTLE_ENDIAN) {
			// little endian, read directly:
			memcpy(&t, (char*)buffer_+pos_, dataSize);
			pos_ += dataSize;
		} else {
			// big endian, must write byte by byte in reverse order
			char* ptr = (char*)&t + dataSize-1;
			while (ptr >= (char*)&t) {
				*(ptr--) = *(((char*)buffer_) + pos_++);
			}
		}
		return *this;
	}

	const BinaryStream& operator >> (std::string &str) const;

protected:
	size_t capacity_ = 0;
	size_t size_ = 0;
	mutable size_t pos_ = 0;
	void *buffer_ = nullptr;
	bool ownsBuffer_ = false;
	std::ifstream *ifstream_ = nullptr;

	void expandBuffer();
};

#endif /* SERIALIZATION_BINARYSTREAM_H_ */
