/*
 * Genome.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#include "Genome.h"
#include "Gene.h"
#include "../utils/rand.h"

Chromosome GeneticOperations::meyosis(const Genome& gen) {
	Chromosome c;
	unsigned i=0;
	while (i<gen.first.size() || i<gen.second.size()) {
		const Gene *g = nullptr;
		if (i<gen.first.size()) {
			g = &gen.first[i];
			if (i<gen.second.size() && randf() < 0.5f)
				g = &gen.second[i];
		} else
			g = &gen.second[i];
		c.push_back(*g);
		i++;
	}
	alterChromosome(c);
	return c;
}

/*
 * 	1. mutating existing genes by altering their data and swapping positions
 * 	2. creating new genes
 * 	3. deleting existing genes
 * 	4. altering the meta-genes for all genes except new ones
 */
void GeneticOperations::alterChromosome(Chromosome &c) {
	// todo start mutating
}

/*
void alter_meta_gene(MetaGene* pMeta)
{
	pMeta->value += srandd() * pMeta->dynamic_variation;
	if (pMeta->value < 0)
		pMeta->value = 0;
}
 */
