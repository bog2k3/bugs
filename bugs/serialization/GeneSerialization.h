/*
 * GeneSerialization.h
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#ifndef SERIALIZATION_GENESERIALIZATION_H_
#define SERIALIZATION_GENESERIALIZATION_H_

class Gene;
class BinaryStream;

BinaryStream& operator << (BinaryStream &stream, Gene const& gene);
BinaryStream& operator >> (BinaryStream &stream, Gene &gene);

#endif /* SERIALIZATION_GENESERIALIZATION_H_ */
