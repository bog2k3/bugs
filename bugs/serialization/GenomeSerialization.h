/*
 * GenomeSerialization.h
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#ifndef SERIALIZATION_GENOMESERIALIZATION_H_
#define SERIALIZATION_GENOMESERIALIZATION_H_

class Genome;
class BinaryStream;

BinaryStream& operator << (BinaryStream &stream, Genome const& genome);
BinaryStream& operator >> (BinaryStream &stream, Genome &genome);

#endif /* SERIALIZATION_GENOMESERIALIZATION_H_ */
