/*
 * BigFile.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: bogdan
 */

#include "BigFile.h"
#include <memory.h>

union bigFile_header {
	struct v1 {
		unsigned version; // assume 1
	};
};

void BigFile::loadFromDisk(const std::string &path) {

}

void BigFile::saveToDisk(const std::string &path) {
	// 1. build header

	// 2. build file table

	// 3. write file data
	for (auto &pair : mapFiles) {
		FileDescriptor &fd = pair.second;
		std::string const &filename = pair.first;
	}
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
