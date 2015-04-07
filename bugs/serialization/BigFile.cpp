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

static constexpr uint32_t BIGFILE_MAGIC = 0xB16F17E5;

struct bigFile_header {
	uint32_t magic = BIGFILE_MAGIC;
	uint32_t version;
	uint32_t reserved[8]; // for future extension
};
BinaryStream& operator << (BinaryStream& stream, bigFile_header const& h) {
	stream << h.magic << h.version;
	for (int i=0; i<8; i++)
		stream << h.reserved[i];
	return stream;
}
BinaryStream& operator >> (BinaryStream& stream, bigFile_header &h) {
	stream >> h.magic >> h.version;
	for (int i=0; i<8; i++)
		stream >> h.reserved[i];
	return stream;
}

bool BigFile::loadFromDisk_v1(BinaryStream &fileStream) {
	bigFile_tableHeader_v1 tableHeader;
	fileStream >> tableHeader;
	std::vector<bigFile_tableEntry_v1> tableEntries;
	for (unsigned i=0; i<tableHeader.numEntries; i++) {
		bigFile_tableEntry_v1 entry;
		fileStream >> entry;
		tableEntries.push_back(entry);
	}
	for (unsigned i=0; i<tableHeader.numEntries; i++) {
		FileDescriptor &fd = mapFiles[tableEntries[i].filename];
		fd.fileName = tableEntries[i].filename;
		fd.size = tableEntries[i].size;
		fd.ownsMemory_ = true;
		fd.pStart = malloc(fd.size);
		#warning "may defer actual malloc and read until the file is requested"
		fileStream.read(fd.pStart, fd.size);
	}
	return true;
}

bool BigFile::saveToDisk_v1(const std::string &path) {
	LOGPREFIX("BigFile")
	// 1. build header
	bigFile_header hdr;
	hdr.version = 1;

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
	LOGPREFIX("BigFile")
	try {
		std::ifstream file(path, std::ios::in | std::ios::binary);
		// void* hdrBinData = malloc(sizeof(bigFile_header));
		// file.read((char*)hdrBinData, sizeof(bigFile_header));
		//BinaryStream hdrStream(hdrBinData, sizeof(bigFile_header));
		BinaryStream fileStream(file);
		bigFile_header hdr;
		fileStream >> hdr;
		if (hdr.magic != BIGFILE_MAGIC) {
			LOGLN("WARNING: Invalid or corrupted BigFile (wrong magic!) at: "<<path);
			return false;
		}
		switch (hdr.version) {
		case 1:
			return loadFromDisk_v1(fileStream);
		default:
			LOGLN("WARNING: No known method to handle version "<<hdr.version<<" of BigFile! canceling...");
			return false;
		}
	} catch (std::ios::failure &e) {
		ERROR("EXCEPTION during loading from disk (" << path<<"):\n" << e.what());
		return false;
	} catch (std::runtime_error &e) {
		ERROR("EXCEPTION during deserialization from file "<< path<<":\n" << e.what());
		return false;
	}
	assert(!!!"should never reach this");
	return false;
}

bool BigFile::saveToDisk(const std::string &path) {
	LOGPREFIX("BigFile")
	saveToDisk_v1(path);
	return true;
}

const BigFile::FileDescriptor BigFile::getFile(const std::string &name) const {
	LOGPREFIX("BigFile")
	auto it = mapFiles.find(name);
	if (it != mapFiles.end())
		return it->second;
	else {
		LOGLN("WARNING: file \""<<name<<"\" doesn't exist in BigFile!!");
		return FileDescriptor();
	}
}

const std::vector<BigFile::FileDescriptor> BigFile::getAllFiles() const {
	std::vector<FileDescriptor> vec;
	for (auto &pair : mapFiles)
		vec.push_back(pair.second);
	return vec;
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
