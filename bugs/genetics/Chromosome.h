/*
 *	a chromosome represents a sequence of genes grouped together.
 *	The chromosome may be inherited as a whole or may be subjected to crossover at gene level
 *	
 *	a chromosome is used to completely describe a single neuron
 */

#ifndef __chromosome_h__
#define __chromosome_h__

#include "constants.h"
#include "../math/tools.h"
#include <vector>
#include "Gene.h"

class Chromosome;
typedef std::vector<Chromosome> Genome;

class Chromosome {
public:
	std::vector<Gene> gene_list;
	MetaGene chance_to_delete; // represents the chromosome's probability to be lost from the genome
	MetaGene chance_to_swap; // chance to swap places with another chromosome
	MetaGene chance_to_split; // this is multiplied by the number of genes in the chromosome,
							// such that the more the chromosome grows, the more likely it is to split
	MetaGene chance_to_spawn_gene; // chance to spawn a new gene in this chromosome

	std::vector<MetaGene*> metaGenes;

	Chromosome()
	: chance_to_delete(constants::initial_chromosome_delete, constants::change_chromosome_delete)
	, chance_to_swap(constants::initial_chromosome_swap, constants::change_chromosome_swap)
	, chance_to_split(constants::initial_chromosome_split, constants::change_chromosome_split)
	, chance_to_spawn_gene(constants::initial_chromosome_spawn_gene, constants::change_chromosome_spawn_gene)
	{
		update_MetaGenes_vec();
	}

	Chromosome(const Chromosome& original)
		: gene_list(original.gene_list)
		, chance_to_delete(original.chance_to_delete)
		, chance_to_swap(original.chance_to_swap)
		, chance_to_split(original.chance_to_split)
		, chance_to_spawn_gene(original.chance_to_spawn_gene)
	{
		update_MetaGenes_vec();
	}

	void add_gene(Gene const &g) {
		gene_list.push_back(g);
	}

private:
	void update_MetaGenes_vec() {
		metaGenes.clear();
		// add meta-genes to this vector to enable their segregation:
		metaGenes.push_back(&chance_to_delete);
		metaGenes.push_back(&chance_to_swap);
		metaGenes.push_back(&chance_to_split);
		metaGenes.push_back(&chance_to_spawn_gene);
	}
};

#endif //__chromosome_h__
