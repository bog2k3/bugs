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

class Serializer {
public:
	Serializer();
	virtual ~Serializer();

	void queueObject(serializable_wrap &&obj);
	void serializeToFile(const std::string &path);

	void setDeserializationObjectMapping(enum objectType objType, std::function<objType?*(binaryStream)> func);
	void deserializeFromFile(const std::string &path); ? return something?
};

#endif /* SERIALIZATION_SERIALIZER_H_ */
