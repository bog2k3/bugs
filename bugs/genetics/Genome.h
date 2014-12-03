/*
 * Genome.h
 *
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#ifndef GENETICS_GENOME_H_
#define GENETICS_GENOME_H_

#include <vector>
#include <utility>

class Gene;

typedef std::vector<Gene> Chromosome;
typedef std::pair<Chromosome, Chromosome> Genome;

class GeneticOperations {
public:
	/**
	 * performs meyosis - creates a new unique Chromosome from the two parent chromosomes, by randomly choosing
	 * one gene from either of them for each position.
	 */
	static Chromosome meyosis(const Genome& gen);

	/**
	 * alters the genome by applying three types of operations:
	 * 	1. mutating existing genes by altering their data and swapping positions
	 * 	2. creating new genes
	 * 	3. altering the meta-genes for all genes except new ones
	 */
	static void alter_genome(Genome &in_out);
};

#endif /* GENETICS_GENOME_H_ */
