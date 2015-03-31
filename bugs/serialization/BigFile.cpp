/*
 * BigFile.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: bogdan
 */

#include "BigFile.h"
#include "BinaryStream.h"
#include "BigFile_v1.h"
#include "../utils/log.h"
#include <memory.h>
#include <stdint.h>
#include <fstream>

bool BigFile::loadFromDisk_v1(const std::string &path) {

}

bool BigFile::saveToDisk_v1(const std::string &path) {
	// 1. build header
	bigFile_header hdr;
	hdr.version = 1;
	hdr.headerSize = sizeof(hdr);

	// 2. build file table
	bigFile_tableHeader_v1 tableHeader;
	tableHeader.numEntries = mapFiles.size();
	BinaryStream tableStream(mapFiles.size() * sizeof(bigFile_tableEntry_v1) * 2);
	unsigned offset = 0;
	for (std::pair<const std::string, FileDescriptor> &pair : mapFiles) {
		FileDescriptor &fd = pair.second;
		bigFile_tableEntry_v1 entry;
		entry.filename = pair.first;
		entry.offset = offset;
		entry.size = fd.size;
		tableStream << entry;
		offset += fd.size;
	}
	// update table header tableSize field:
	tableHeader.tableSize = tableStream.getSize();

	// 3. serialize header & table header:
	BinaryStream headerAndTableStream(sizeof(hdr)+sizeof(tableHeader));
	headerAndTableStream << hdr << tableHeader;

	// 4. write file data
	try {
		std::ofstream file(path, std::ios::out | std::ios::binary);
		file.write((const char*)headerAndTableStream.getBuffer(), headerAndTableStream.getSize());
		file.write((const char*)tableStream.getBuffer(), tableStream.getSize());
		for (auto &pair : mapFiles) {
			FileDescriptor &fd = pair.second;
			file.write((const char*)fd.pStart, fd.size);
		}
		file.close();
	} catch (std::ios::failure &e) {
		ERROR(e.what());
		return false;
	}
	return true;
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
