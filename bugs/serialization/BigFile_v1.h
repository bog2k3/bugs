/*
 * BigFile_v1.h
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#ifndef SERIALIZATION_BIGFILE_V1_H_
#define SERIALIZATION_BIGFILE_V1_H_

#include "BigFile_magic.h"

struct bigFile_header {
	uint32_t magic = BIGFILE_MAGIC;
	uint32_t version;
	uint32_t headerSize;
	uint32_t reserved[8]; // for future extension
};
BinaryStream& operator << (BinaryStream& stream, bigFile_header const& h) {
	stream << h.magic << h.version << h.headerSize;
	for (int i=0; i<8; i++)
		stream << h.reserved[i];
	return stream;
}
BinaryStream& operator >> (BinaryStream& stream, bigFile_header &h) {
	stream >> h.magic >> h.version >> h.headerSize;
	for (int i=0; i<8; i++)
		stream >> h.reserved[i];
	return stream;
}

struct bigFile_tableHeader_v1 {
	uint32_t tableSize;
	uint32_t numEntries;
	uint32_t reserved[8]; // for future extension
};
BinaryStream& operator << (BinaryStream& stream, bigFile_tableHeader_v1 const& h) {
	stream << h.tableSize << h.numEntries;
	for (int i=0; i<8; i++)
		stream << h.reserved[i];
	return stream;
}
BinaryStream& operator >> (BinaryStream& stream, bigFile_tableHeader_v1 &h) {
	stream >> h.tableSize >> h.numEntries;
	for (int i=0; i<8; i++)
		stream >> h.reserved[i];
	return stream;
}

struct bigFile_tableEntry_v1 {
	std::string filename;
	uint32_t offset;	// offset from the beginning of the contents; first file body has offset 0
	uint32_t size;
	uint32_t reserved[2]; // for future extension
};
BinaryStream& operator << (BinaryStream& stream, bigFile_tableEntry_v1 const& e) {
	stream << e.filename << e.offset << e.size;
	for (int i=0; i<2; i++)
		stream << e.reserved[i];
	return stream;
}
BinaryStream& operator >> (BinaryStream& stream, bigFile_tableEntry_v1 &e) {
	stream >> e.filename >> e.offset >> e.size;
	for (int i=0; i<2; i++)
		stream >> e.reserved[i];
	return stream;
}

#endif /* SERIALIZATION_BIGFILE_V1_H_ */
