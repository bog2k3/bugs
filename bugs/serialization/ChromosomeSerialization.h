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
StreamType& operator << (StreamType &stream, Chromosome const& chromosome) {
	stream << (uint32_t)chromosome.genes.size();
	for (unsigned i=0; i<chromosome.genes.size(); i++)
		stream << chromosome.genes[i];
	return stream;
}

template<class StreamType>
StreamType& operator >> (StreamType &stream, Chromosome &chromosome) {
	chromosome.genes.clear();
	uint32_t nGenes;
	stream >> nGenes;
	chromosome.genes.reserve(nGenes);
	for (unsigned i=0; i<nGenes; i++) {
		Gene g;
		stream >> g;
		chromosome.genes.push_back(g);
	}
	return stream;
}

static size_t dataSize(Chromosome const& c) {
	size_t sz = sizeof(uint32_t); // length of genes array
	for (auto &g : c.genes)
		sz += dataSize(g);
	return sz;
}

#endif /* SERIALIZATION_CHROMOSOMESERIALIZATION_H_ */
