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

static constexpr uint32_t BIGFILE_MAGIC = 0xB16F17E69;

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

bool BigFile::loadFromDisk_v1(std::ifstream &file) {

}

bool BigFile::saveToDisk_v1(const std::string &path) {
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
	try {
		std::ifstream file(path, std::ios::in | std::ios::binary);
		void* hdrBinData = malloc(sizeof(bigFile_header));
		file.read(hdrBinData, sizeof(bigFile_header));
		BinaryStream hdrStream(hdrBinData, sizeof(bigFile_header));
		bigFile_header hdr;
		if (hdr.magic != BIGFILE_MAGIC) {
			LOGLN("WARNING: Invalid or corrupted BigFile (wrong magic!) at: "<<path);
			return false;
		}
		switch (hdr.version) {
		case 1:
			return loadFromDisk_v1(file);
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
	return true;
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
