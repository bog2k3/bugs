/*
 * BigFile.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: bogdan
 */

#include "BigFile.h"
#include <memory.h>
#include <stdint.h>

struct bigFile_header {
	uint32_t version;
	uint32_t headerSize;
	union VERSIONS {
		struct V1 {
			uint32_t tableSize;
			uint32_t numEntries;
			uint32_t maxEntrySize;
			uint32_t reserved[8]; // for future extension
		} v1;
	} headerData;
};

void BigFile::loadFromDisk_v1(const std::string &path) {

}

void BigFile::saveToDisk_v1(const std::string &path) {
	// 1. build header
	bigFile_header hdr;
	hdr.version = 1;
	hdr.headerSize = sizeof(hdr.version) + sizeof(hdr.headerSize) + sizeof(hdr.headerData.v1);

	// 2. build file table

	// 3. write file data
	for (auto &pair : mapFiles) {
		FileDescriptor &fd = pair.second;
		std::string const &filename = pair.first;
	}
}

void BigFile::loadFromDisk(const std::string &path) {

}

void BigFile::saveToDisk(const std::string &path) {
	saveToDisk_v1(path);
}

BigFile::FileDescriptor BigFile::getFile(const std::string &name) {

}

std::vector<BigFile::FileDescriptor> BigFile::getAllFiles() {

}

void BigFile::addFile(const std::string &filename, void* buffer, size_t size) {
	FileDescriptor &fd = mapFiles[filename] = FileDescriptor();
	fd.fileName = filename;
	fd.ownsMemory_ = true;
	fd.pStart = malloc(size);
	memcpy(fd.pStart, buffer, size);
	fd.size = size;
}

void BigFile::extractAll(const std::string &pathOut) {

}

void BigFile::extractFile(const std::string &pathOut, const std::string &filename) {

}
