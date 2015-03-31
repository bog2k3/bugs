/*
 * ChromosomeSerialization.h
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#ifndef SERIALIZATION_CHROMOSOMESERIALIZATION_H_
#define SERIALIZATION_CHROMOSOMESERIALIZATION_H_

struct Chromosome;
class BinaryStream;

BinaryStream& operator << (BinaryStream &stream, Chromosome const& chromosome);
BinaryStream& operator >> (BinaryStream &stream, Chromosome &chromosome);

#endif /* SERIALIZATION_CHROMOSOMESERIALIZATION_H_ */
