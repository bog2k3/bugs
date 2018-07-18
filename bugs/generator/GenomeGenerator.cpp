/*
 * GenomeGenerator.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#include "GenomeGenerator.h"
#include "../genetics/Gene.h"

Genome GenomeGenerator::createRandom(int length) {
	Genome genome;
	for (int i=0; i<length; i++) {
		Gene g = Gene::createRandom(length-i-1);
		genome.first.genes.push_back(g);
		GeneticOperations::alterGene(g, 1.f);
		genome.second.genes.push_back(g);
	}
	return genome;
}
