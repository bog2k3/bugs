/*
 *	a gene is the fundamental genetic unit of information. It cannot be subdivided
 *	Genes may be altered by mutation but they are always inherited as a whole.
 */

#ifndef __gene_h__
#define __gene_h__

#include "../math/tools.h"
#include "constants.h"
#include <map>
#include <vector>

enum gene_type {
	GENE_TYPE_INVALID = 0,

	GENE_INPUT_SOURCE,			// data-type: int; specifies the index of a neuron to use as input or index of default inputs if negative
	GENE_INPUT_WEIGHT,			// data-type: double; specifies weight for an input (in the order of occurence)
	GENE_BIAS,					// data-type: double; specifies bias for neural function
	GENE_TRANSFER_FUNCTION,		// data-type: int; specifies the transfer function
	GENE_TRANSFER_ARGUMENT,		// data-type: double; specifies an extra argument to be passed to the transfer function
	GENE_OUTPUT,				// data-type: double; indicates that the affected neuron should be linked to the network's output;
								// the value of this gene is a random between [0..1] and in relation with other output neurons' gene's value will
								// determine the order in which the output neurons are connected to network output sockets (from smallest
								// to largest value). The number of output neurons in a network is always fixed, thus
								// the output genes have a special regime, they are 100% stable to deletion and transduction and a fixed number
								// of them must be present in each genome, equal to the network's output count.

	GENE_TYPE_END
};

enum gene_value_type {
	GENE_VALUE_INVALID = 0,

	GENE_VALUE_INT,
	GENE_VALUE_DOUBLE,

	GENE_VALUE_END
};

// need to map each gene type to a value-type, in order to perform meaningful gene transformations and spawning

class MetaGene {
public:
	double value;
	double dynamic_variation;

	MetaGene(double initial_reference_value, double dynamic_variation)
		: value (initial_reference_value * randd())
		, dynamic_variation(dynamic_variation)
	{ }

	MetaGene()
		: value(0), dynamic_variation(0)
	{ }
};

struct Gene {
	Gene(gene_type type, int value_int, double value_double)
		: type(type)
		, value_int(value_int)
		, value_double(value_double)
		, chance_to_mutate(constants::initial_gene_mutate, constants::change_gene_mutate)
		, chance_to_transform(constants::initial_gene_transform, constants::change_gene_transform)
		, chance_to_delete(constants::initial_gene_delete, constants::change_gene_delete)
		, chance_to_swap(constants::initial_gene_swap, constants::change_gene_swap)
		, mutation_reference_value(constants::initial_gene_mutation_value, constants::change_gene_mutation_value)
	{
		value_type = mapGeneValueTypes[type];
		update_meta_genes_vec();
	}

	Gene(const Gene& original)
		: type(original.type)
		, value_int(original.value_int)
		, value_double(original.value_double)
		, chance_to_mutate(original.chance_to_mutate)
		, chance_to_transform(original.chance_to_transform)
		, chance_to_delete(original.chance_to_delete)
		, chance_to_swap(original.chance_to_swap)
		, mutation_reference_value(original.mutation_reference_value)
	{
		value_type = mapGeneValueTypes[type];
		update_meta_genes_vec();
	}

	Gene()
		: type(GENE_TYPE_INVALID)
	{
		update_meta_genes_vec();
	}

	static std::map<gene_type, gene_value_type> mapGeneValueTypes;

	gene_type type;
	gene_value_type value_type;
	int value_int;
	double value_double;

	std::vector<MetaGene*> metaGenes;

	MetaGene chance_to_mutate;	// [0..1] represents the likelihood that this gene will mutate (value-wise, keeps type)
	MetaGene chance_to_transform;	// [0..1] chance to transform into another gene type (keeps value, modifies type)
	MetaGene chance_to_delete;	// [0..1] represents the likelihood that this gene will disappear completely
	MetaGene chance_to_swap;		// likelihood that this gene will swap places with another gene

	MetaGene mutation_reference_value; // maximum value by which a gene can be mutated. The mutation is random between - and + this value

private:
	void update_meta_genes_vec() {
		metaGenes.clear();
		// add meta-genes to this vector to enable their segregation:
		metaGenes.push_back(&chance_to_mutate);
		metaGenes.push_back(&chance_to_transform);
		metaGenes.push_back(&chance_to_delete);
		metaGenes.push_back(&chance_to_swap);

		// special case: output genes are 100% stable to deletion and transformation:
		if (type == GENE_OUTPUT) {
			chance_to_transform.value = 0;
			chance_to_transform.dynamic_variation = 0;
			chance_to_delete.value = 0;
			chance_to_delete.dynamic_variation = 0;
		}
		if (value_type == GENE_VALUE_INT)
			mutation_reference_value.value *= 10;
	}
};

#endif //__gene_h__
