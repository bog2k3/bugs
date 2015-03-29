/*
 * Serializer.h
 *
 *  Created on: Mar 29, 2015
 *      Author: bog
 */

#ifndef SERIALIZATION_SERIALIZER_H_
#define SERIALIZATION_SERIALIZER_H_

#include "serializable.h"
#include <string>
#include <functional>
#include <map>
#include <vector>

class Serializer {
public:
	Serializer();
	virtual ~Serializer();

	typedef std::function<void(BinaryStream &stream)> DeserializeFuncType;

	void queueObject(serializable_wrap &&obj);
	bool serializeToFile(const std::string &path);

	void setDeserializationObjectMapping(SerializationObjectTypes objType, DeserializeFuncType func);
	bool deserializeFromFile(const std::string &path);

private:
	std::vector<serializable_wrap> serializationQueue_;
	std::map<SerializationObjectTypes, DeserializeFuncType> mapTypesToFuncs_;

	std::string getObjectTypeString(SerializationObjectTypes type);
};

#endif /* SERIALIZATION_SERIALIZER_H_ */
