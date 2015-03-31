/*
 * Serializer.cpp
 *
 *  Created on: Mar 29, 2015
 *      Author: bog
 */

#include "Serializer.h"
#include "BigFile.h"
#include "BinaryStream.h"
#include <memory>
#include <sstream>

Serializer::Serializer() {
}

Serializer::~Serializer() {
}

BinaryStream& operator << (BinaryStream &stream, SerializationObjectTypes x) {
	stream << (uint16_t)x;
	return stream;
}

BinaryStream& operator >> (BinaryStream &stream, SerializationObjectTypes x) {
	stream >> (uint16_t&)x;
	return stream;
}


void Serializer::queueObject(serializable_wrap &&obj) {
	serializationQueue_.push_back(obj);
}

std::string Serializer::getObjectTypeString(SerializationObjectTypes type) {
	switch (type) {
	case SerializationObjectTypes::BUG:
		return "bug";
	case SerializationObjectTypes::GAMETE:
		return "gamete";
	case SerializationObjectTypes::GENOME:
		return "genome";
	default:
		return "UNKNOWN_TYPE";
	}
}

#warning "add logs for all types of errors"

bool Serializer::serializeToFile(const std::string &path) {
	BinaryStream masterStream(serializationQueue_.size() * 50); // estimate about 50 bytes per entry in master
	std::vector<std::unique_ptr<BinaryStream>> vecStreams;
	std::vector<std::string> vecFilenames;
	int fileIndex = 1;
	for (auto &e : serializationQueue_) {
		SerializationObjectTypes objType = e.getType();
		assert(objType != SerializationObjectTypes::UNDEFINED);
		masterStream << objType;
		std::stringstream pathBuild;
		pathBuild << getObjectTypeString(e.getType()) << fileIndex << ".data";
		vecFilenames.push_back(pathBuild.str());
		masterStream << pathBuild.str();
		std::unique_ptr<BinaryStream> fileStream(new BinaryStream(100));
		e.serialize(*fileStream);
		vecStreams.push_back(std::move(fileStream));

		fileIndex++;
	}
	serializationQueue_.clear();
	BigFile bigFile;
	bigFile.addFile("master", masterStream.getBuffer(), masterStream.getSize());
	for (unsigned i=0; i<vecStreams.size(); i++) {
		bigFile.addFile(vecFilenames[i], vecStreams[i]->getBuffer(), vecStreams[i]->getSize());
		vecStreams[i].reset();
	}
	return bigFile.saveToDisk(path);
}

void Serializer::setDeserializationObjectMapping(SerializationObjectTypes objType, DeserializeFuncType func) {
	mapTypesToFuncs_[objType] = func;
}

bool Serializer::deserializeFromFile(const std::string &path) {
	BigFile bigFile;
	if (!bigFile.loadFromDisk(path))
		return false;
	BigFile::FileDescriptor master = bigFile.getFile("master");
	if (master.pStart == nullptr)
		return false;
	BinaryStream masterStream(master.pStart, master.size);
	while (!masterStream.eof()) {
		SerializationObjectTypes type;
		std::string filename;
		masterStream >> type >> filename;
		DeserializeFuncType deserializeFunc = mapTypesToFuncs_[type];
		if (!deserializeFunc)
			return false;
		BigFile::FileDescriptor fileDesc = bigFile.getFile(filename);
		if (!fileDesc.pStart)
			return false; // or continue and log warning?
		BinaryStream fileStream(fileDesc.pStart, fileDesc.size);
		deserializeFunc(fileStream);
	}
	return true;
}

