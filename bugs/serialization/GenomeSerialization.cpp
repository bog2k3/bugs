/*
 * GenomeSerialization.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#include "../genetics/Genome.h"
#include "BinaryStream.h"
#include "ChromosomeSerialization.h"

BinaryStream& operator << (BinaryStream &stream, Genome const& genome) {
	stream << genome.first;
	stream << genome.second;
	return stream;
}

BinaryStream& operator >> (BinaryStream &stream, Genome &genome) {
	stream >> genome.first;
	stream >> genome.second;
	return stream;
}

