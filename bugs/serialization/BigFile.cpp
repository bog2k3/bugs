/*
 * BigFile.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: bogdan
 */

#include "BigFile.h"
#include <memory.h>
#include <stdint.h>
#include <fstream>

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

bool BigFile::loadFromDisk_v1(const std::string &path) {

}

bool BigFile::saveToDisk_v1(const std::string &path) {
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

bool BigFile::loadFromDisk(const std::string &path) {
	std::ifstream file(path, std::ios::in | std::ios::binary);
	bigFile_header hdr;
	file >> hdr.version;
	file >> hdr.headerSize;
}

bool BigFile::saveToDisk(const std::string &path) {
	saveToDisk_v1(path);
}

const BigFile::FileDescriptor BigFile::getFile(const std::string &name) const {

}

const std::vector<BigFile::FileDescriptor> BigFile::getAllFiles() const {

}

void BigFile::addFile(const std::string &filename, const void* buffer, size_t size) {
	FileDescriptor &fd = mapFiles[filename] = FileDescriptor(size, malloc(size), filename);
	fd.ownsMemory_ = true;
	memcpy(fd.pStart, buffer, size);
}

void BigFile::extractAll(const std::string &pathOut) {

}

void BigFile::extractFile(const std::string &pathOut, const std::string &filename) {

}
