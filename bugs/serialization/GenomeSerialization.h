/*
 * GenomeSerialization.h
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#ifndef SERIALIZATION_GENOMESERIALIZATION_H_
#define SERIALIZATION_GENOMESERIALIZATION_H_

#include "ChromosomeSerialization.h"
#include "../genetics/Genome.h"

template<class StreamType>
StreamType& operator << (StreamType &stream, Genome const& genome) {
	stream << genome.first;
	stream << genome.second;
	return stream;
}

template<class StreamType>
StreamType& operator >> (StreamType &stream, Genome &genome) {
	stream >> genome.first;
	stream >> genome.second;
	return stream;
}

#endif /* SERIALIZATION_GENOMESERIALIZATION_H_ */
