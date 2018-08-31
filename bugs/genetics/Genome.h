/*
 * Genome.h
 *
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#ifndef GENETICS_GENOME_H_
#define GENETICS_GENOME_H_

#include "constants.h"
#include "Gene.h"

#include <vector>
#include <string>
#include <utility>

class MetaGene;

struct Chromosome {
	std::vector<Gene> genes;

	bool isGeneticallyCompatible(Chromosome const& c) {
		return (unsigned)abs((int)c.genes.size() - (int)genes.size()) <= constants::MaxGenomeLengthDifference;
	}

	bool operator == (Chromosome const&) const;

	std::string stringify() const;
};

typedef std::pair<Chromosome, Chromosome> Genome;

class GeneticOperations {
public:
	/**
	 * performs meyosis - creates a new unique Chromosome from the two parent chromosomes, by randomly choosing
	 * one gene from either of them for each position.
	 * also alters the resulting chromosome by applying these types of operations:
	 * 	1. mutating existing genes by altering their data and swapping positions
	 * 	2. creating new genes
	 * 	3. deleting existing genes
	 * 	4. altering the meta-genes for all genes except new ones
	 */
	static Chromosome meyosis(const Genome& gen);

	static void alterChromosome(Chromosome &c);

	// return number of mutated atoms
	static int alterGene(Gene &g, float mutationChanceFactor);

private:
	static void getAlterationChances(Gene const& g, float& mutationCh, float& swapCh, float& deleteCh);
	static void alterMetaGene(MetaGene &m);
	static void insertNewGene(Chromosome &c, int index, Gene const& g);
};

#endif /* GENETICS_GENOME_H_ */
