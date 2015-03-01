/*
 * Genome.h
 *
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#ifndef GENETICS_GENOME_H_
#define GENETICS_GENOME_H_

#include "../entities/WorldConst.h"
#include <vector>
#include <utility>

class Gene;
class MetaGene;

struct Chromosome {
	std::vector<Gene> genes;
	int lastInsertPos[WorldConst::MaxGenomeLengthDifference] { -1 };
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

private:
	static float getTotalMutationChance(Gene const& g);
	// return number of mutated atoms
	static int alterGene(Gene &g, float mutationChanceFactor);
	static void alterMetaGene(MetaGene &m);
};

#endif /* GENETICS_GENOME_H_ */
