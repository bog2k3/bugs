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
	GENE_TYPE_DEVELOPMENT,		// developmental gene - commands the growth of the body
	GENE_TYPE_PART_ATTRIBUTE,	// body part attribute - establishes characteristics of certain body parts
	GENE_TYPE_GENERAL_ATTRIB,	// general body attribute - controls the overall features of all
								//		body parts of a specific type.
	GENE_TYPE_NEURON,			// creates a new neuron
	GENE_TYPE_SYNAPSE,			// creates a synapse between neurons
	GENE_TYPE_TRANSFER,			// controls the transfer function of a neuron
	GENE_TYPE_MUSCLE_COMMAND,	// creates a linkage from a neuron to a command neuron that sends signals to muscles
	GENE_TYPE_END
};

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

/**
 * This holds an atomic value of whatever type, that is subject to mutation.
 */
template<typename T>
struct Atom {
	T value;
	MetaGene meta;

	operator T() { return value; }
	void set(T value, double meta_ref_value, double meta_dynamic_var) {
		this->value = value;
		this->meta.value = meta_ref_value;
		this->meta.dynamic_variation = meta_dynamic_var;
	}
};

struct GeneCommand {
	Atom<unsigned> location;
	int command;
	int part_type;
	Atom<float> angle;
};

struct GeneLocalAttribute {
	Atom<unsigned> location;
	int attribute;
	bool relativeValue;
	Atom<float> value;
};

struct GeneGeneralAttribute {
	int attribute;
	Atom<float> value;					// this is always relative
};

struct GeneSynapse {
	Atom<int> delta;			// neuron index delta from the current neuron
	Atom<float> weight;
};

struct GeneTransferFunction {
	Atom<int> functionID;
};

struct GeneMuscleCommand {
	Atom<int> muscleID;
	Atom<int> neuronDelta;
	Atom<float> weight;
};

class Gene {
public:
	int RID;			// this is unique to this gene, when the gene is mutated, the RID changes
	gene_type type;		// the type of gene
	union GeneData {
		GeneCommand gene_command;
		GeneLocalAttribute gene_local_attribute;
		GeneGeneralAttribute gene_general_attribute;
		GeneSynapse gene_synapse;
		GeneTransferFunction gene_transfer_function;
		GeneMuscleCommand gene_muscle_command;
	} data;

	Gene(gene_type type, GeneData &data)
		: type(type)
		, data(data)
		, chance_to_delete(constants::initial_gene_delete, constants::change_gene_delete)
		, chance_to_swap_ahead(constants::initial_gene_swap, constants::change_gene_swap)
		, chance_to_swap_behind(constants::initial_gene_swap, constants::change_gene_swap)
		, mutation_reference_value(constants::initial_gene_mutation_value, constants::change_gene_mutation_value)
	{
		update_meta_genes_vec();
	}

	Gene(const Gene& original)
		: type(original.type)
		, data(original.data)
		, chance_to_delete(original.chance_to_delete)
		, chance_to_swap_ahead(original.chance_to_swap_ahead)
		, chance_to_swap_behind(original.chance_to_swap_behind)
		, mutation_reference_value(original.mutation_reference_value)
	{
		update_meta_genes_vec();
	}

	std::vector<MetaGene*> metaGenes;

	MetaGene chance_to_delete;		// [0..1] represents the likelihood that this gene will disappear completely
	MetaGene chance_to_swap_ahead;	// likelihood that this gene will swap places with the next one
	MetaGene chance_to_swap_behind;	// likelihood that this gene will swap places with the previous one

	MetaGene mutation_reference_value; // maximum value by which a gene can be mutated. The mutation is random between - and + this value

private:
	void update_meta_genes_vec();
};

#endif //__gene_h__
