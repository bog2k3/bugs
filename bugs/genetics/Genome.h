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
#include <string>
#include <utility>

class MetaGene;

struct Chromosome {
	std::vector<Gene> genes;
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

	bool isGeneticallyCompatible(Chromosome const& c) {
		return (unsigned)abs((int)c.genes.size() - (int)genes.size()) <= WorldConst::MaxGenomeLengthDifference;
	}

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

	/**
	 * fixes genes synchronization for a new genome created out of two chromosomes that came from gamettes.
	 */
	static void fixGenesSynchro(Genome& gen);

private:
	static void getAlterationChances(Gene const& g, float& mutationCh, float& swapCh, float& deleteCh);
	static void alterMetaGene(MetaGene &m);
	static void pullBackInsertions(Chromosome &c, int amount);
	static int insertNewGene(Chromosome &c, Chromosome::insertion ins, Gene const& g);
	static void trimInsertionList(Chromosome &c);
};

#endif /* GENETICS_GENOME_H_ */
