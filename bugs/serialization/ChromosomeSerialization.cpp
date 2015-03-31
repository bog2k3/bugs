/*
 * ChromosomeSerialization.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#include "BinaryStream.h"
#include "GeneSerialization.h"
#include "../genetics/Genome.h"

BinaryStream& operator << (BinaryStream &stream, Chromosome::insertion const& ins) {
	stream << ins.age << ins.index;
	return stream;
}

BinaryStream& operator >> (BinaryStream &stream, Chromosome::insertion &ins) {
	stream >> ins.age >> ins.index;
	return stream;
}

BinaryStream& operator << (BinaryStream &stream, Chromosome const& chromosome) {
	stream << (uint32_t)chromosome.genes.size();
	for (Gene const &g : chromosome.genes)
		stream << g;
	stream << (uint16_t)chromosome.insertions.size();
	for (Chromosome::insertion const &i : chromosome.insertions)
		stream << i;
	return stream;
}

BinaryStream& operator >> (BinaryStream &stream, Chromosome &chromosome) {
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
	chromosome.insertions.reserve(nInsertions);
	for (unsigned i=0; i<nInsertions; i++) {
		Chromosome::insertion ins;
		stream >> ins;
		chromosome.insertions.push_back(ins);
	}
	return stream;
}
