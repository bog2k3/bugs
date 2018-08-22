/*
 * GenomeGenerator.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#include "GenomeGenerator.h"
#include "../genetics/Gene.h"

#include <boglfw/utils/assert.h>

Genome GenomeGenerator::createRandom(int length) {
#ifdef DEBUG
	assertDbg(length > 20); // what are you trying to do creating these tiny ass genomes???
#endif
	Genome genome;
	for (int i=0; i<length; i++) {
		Gene g = Gene::createRandom(length-i-1);
		genome.first.genes.push_back(g);
		GeneticOperations::alterGene(g, 1.f);
		genome.second.genes.push_back(g);
	}
	return genome;
}
