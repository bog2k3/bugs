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

	virtual ~BinaryStream();

	size_t getCapacity() const { return capacity_; }
	size_t getSize() const { return size_; }
	const void* getBuffer() const { return buffer_; }
	void seek(size_t offset) const;

	/**
	 * inputs data into the stream, incrementing the position. If the size of the stream grows larger than its capacity:
	 * 	a. the capacity is increased if the stream owns the underlying buffer
	 * 	OR:
	 * 	b. an exception is generated if the underlying buffer is not owned by the stream.
	 */
	template<typename T, typename std::enable_if<std::is_fundamental<T>::value>::type>
	BinaryStream& operator << (T& t);

	/**
	 * outputs data from the stream, incrementing the position.
	 * If attempting to read past the end of the stream, an exception is generated.
	 */
	template<typename T, typename std::enable_if<std::is_fundamental<T>::value>::type>
	BinaryStream& operator >> (T& t) const;

protected:
	size_t capacity_ = 0;
	size_t size_ = 0;
	mutable size_t pos_ = 0;
	void *buffer_ = nullptr;
	bool ownsBuffer_ = false;

	void expandBuffer();
};

#endif /* SERIALIZATION_BINARYSTREAM_H_ */
