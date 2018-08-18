/*
 * ChromosomeSerialization.h
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#ifndef SERIALIZATION_CHROMOSOMESERIALIZATION_H_
#define SERIALIZATION_CHROMOSOMESERIALIZATION_H_

#include "GeneSerialization.h"
#include "../genetics/Genome.h"

template<class StreamType>
StreamType& operator << (StreamType &stream, Chromosome::insertion const& ins) {
	stream << ins.age << ins.index;
	return stream;
}

template<class StreamType>
StreamType& operator >> (StreamType &stream, Chromosome::insertion &ins) {
	stream >> ins.age >> ins.index;
	return stream;
}

template<class StreamType>
StreamType& operator << (StreamType &stream, Chromosome const& chromosome) {
	stream << (uint32_t)chromosome.genes.size();
	for (unsigned i=0; i<chromosome.genes.size(); i++)
		stream << chromosome.genes[i];
	stream << (uint16_t)chromosome.insertions.size();
	for (unsigned i=0; i<chromosome.insertions.size(); i++)
		stream << chromosome.insertions[i];
	return stream;
}

template<class StreamType>
StreamType& operator >> (StreamType&stream, Chromosome &chromosome) {
	chromosome.genes.clear();
	chromosome.insertions.clear();
	uint32_t nGenes;
	stream >> nGenes;
	chromosome.genes.reserve(nGenes);
	for (unsigned i=0; i<nGenes; i++) {
		Gene g;
		stream >> g;
		chromosome.genes.push_back(g);
	}
	uint16_t nInsertions;
	stream >> nInsertions;
	nInsertions = min(nInsertions, (uint16_t)constants::MaxGenomeLengthDifference);
	chromosome.insertions.reserve(nInsertions);
	for (unsigned i=0; i<nInsertions; i++) {
		Chromosome::insertion ins;
		stream >> ins;
		chromosome.insertions.push_back(ins);
	}
	return stream;
}

#endif /* SERIALIZATION_CHROMOSOMESERIALIZATION_H_ */
