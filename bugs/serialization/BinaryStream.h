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

	BinaryStream& operator << (int8_t const& i);
	BinaryStream& operator << (uint8_t const& i);
	BinaryStream& operator << (int16_t const& i);
	BinaryStream& operator << (uint16_t const& i);
	BinaryStream& operator << (int32_t const& i);
	BinaryStream& operator << (uint32_t const& i);
	BinaryStream& operator << (int64_t const& i);
	BinaryStream& operator << (uint64_t const& i);
	BinaryStream& operator << (float const& f);
	BinaryStream& operator << (const std::string &str);

	BinaryStream& operator >> (int8_t &i);
	BinaryStream& operator >> (uint8_t &i);
	BinaryStream& operator >> (int16_t &i);
	BinaryStream& operator >> (uint16_t &i);
	BinaryStream& operator >> (int32_t &i);
	BinaryStream& operator >> (uint32_t &i);
	BinaryStream& operator >> (int64_t &i);
	BinaryStream& operator >> (uint64_t &i);
	BinaryStream& operator >> (float &f);
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

	template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
	BinaryStream& operator << (T const& t);

	template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
	BinaryStream& operator >> (T &t);
};

#endif /* SERIALIZATION_BINARYSTREAM_H_ */
