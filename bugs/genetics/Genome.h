/*
 * Genome.h
 *
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#ifndef GENETICS_GENOME_H_
#define GENETICS_GENOME_H_

#include "../entities/WorldConst.h"
#include "Gene.h"
#include <vector>
#include <utility>

class MetaGene;

struct Chromosome {
	std::vector<Gene> genes;
	// int lastInsertPos[WorldConst::MaxGenomeLengthDifference] { -1 };	// [0] is the oldest addition, highest index non -1 is the most recent
	struct insertion {
		int index = -1;
		int age = 0;	// in number of generations

		insertion() = default;
		insertion(int index, int age) : index(index), age(age) {}
	};
	std::vector<insertion> insertions;

	Chromosome() : insertions(WorldConst::MaxGenomeLengthDifference, insertion()) {
		insertions.clear();
	}
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

	/**
	 * fixes genes synchronization for a new genome created out of two chromosomes that came from gamettes.
	 */
	static void fixGenesSynchro(Genome& gen);

private:
	static float getTotalMutationChance(Gene const& g);
	// return number of mutated atoms
	static int alterGene(Gene &g, float mutationChanceFactor);
	static void alterMetaGene(MetaGene &m);
	static void pullBackInsertions(Chromosome &c, int amount);
	static void insertNewGene(Chromosome &c, Chromosome::insertion ins, Gene const& g);
	static void trimInsertionList(Chromosome &c);
};

#endif /* GENETICS_GENOME_H_ */
