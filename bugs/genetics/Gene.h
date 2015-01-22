/*
 *	a gene is the fundamental genetic unit of information. It cannot be subdivided
 *	Genes may be altered by mutation but they are always inherited as a whole.
 */

#ifndef __gene_h__
#define __gene_h__

#include "../utils/rand.h"
#include "constants.h"
#include "GeneDefinitions.h"
#include <stdint.h>
#include <map>
#include <vector>

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

	operator T() const { return value; }
	void set(T value) {
		this->value = value;
		this->meta.value = constants::initial_gene_mutate;
		this->meta.dynamic_variation = constants::change_gene_mutate;
	}

	Atom() : value(), meta() {}
};

typedef uint16_t LocationLevelType; // bit 15 = current node, bits 0..14 children nodes

struct GeneLocation {
	Atom<LocationLevelType> location[constants::MAX_GROWTH_DEPTH];
#warning "when mutating this gene, use only bit 15 and 0 for odd levels, and all 16 for even levels"
};

struct GeneCommand {
	gene_development_command command;
	gene_part_type part_type;
	Atom<float> angle;			// angle is relative to the previous element's orientation
};

struct GeneLocalAttribute {
	Atom<float> value;
	gene_part_attribute_type attribute;
	bool relativeValue;
};

struct GeneGeneralAttribute {
	gene_part_attribute_type attribute;
	Atom<float> value;					// this is always relative
};

struct GeneSynapse {
	Atom<int> from;		// negative means sensor, positive or 0 means neuron index
	Atom<int> to;		// negative means motor, positive or 0 means neuron index
	Atom<float> weight;
};

struct GeneFeedbackSynapse {
	Atom<int> from;		// always positive - index of motor command neuron
	Atom<int> to;		// negative means motor, positive or 0 means neuron index
	Atom<float> weight;
};

struct GeneTransferFunction {
	Atom<int> targetNeuron;
	Atom<int> functionID;
};

struct GeneNeuralConstant {
	Atom<int> targetNeuron;
	Atom<float> value;
};

struct GeneBodyAttribute {
	gene_body_attribute_type attribute;
	Atom<float> value;
};

class Gene {
public:
	unsigned long RID;	// this is unique to this gene, when the gene is mutated, the RID changes
	gene_type type;		// the type of gene
	union GeneData {
		GeneLocation gene_location;
		GeneCommand gene_command;
		GeneLocalAttribute gene_local_attribute;
		GeneGeneralAttribute gene_general_attribute;
		GeneSynapse gene_synapse;
		GeneTransferFunction gene_transfer_function;
		GeneNeuralConstant gene_neural_constant;
		GeneFeedbackSynapse gene_feedback_synapse;
		GeneBodyAttribute gene_body_attribute;

		GeneData(GeneLocation const &gl) : gene_location(gl) {}
		GeneData(GeneCommand const &gc) : gene_command(gc) {}
		GeneData(GeneLocalAttribute const &gla) : gene_local_attribute(gla) {}
		GeneData(GeneGeneralAttribute const &gga) : gene_general_attribute(gga) {}
		GeneData(GeneSynapse const &gs) : gene_synapse(gs) {}
		GeneData(GeneFeedbackSynapse const &gfs) : gene_feedback_synapse(gfs) {}
		GeneData(GeneTransferFunction const &gt) : gene_transfer_function(gt) {}
		GeneData(GeneNeuralConstant const &gnc) : gene_neural_constant(gnc) {}
		GeneData(GeneBodyAttribute const &gba) : gene_body_attribute(gba) {}
	} data;

	Gene(gene_type type, GeneData data)
		: RID(new_RID())
		, type(type)
		, data(data)
		, chance_to_delete(constants::initial_gene_delete, constants::change_gene_delete)
		, chance_to_swap(constants::initial_gene_swap, constants::change_gene_swap)
		, mutation_reference_value(constants::initial_gene_mutation_value, constants::change_gene_mutation_value)
	{
		update_meta_genes_vec();
	}

	Gene(GeneLocation const &gl) : Gene(GENE_TYPE_LOCATION, gl) {}
	Gene(GeneCommand const &gc) : Gene(GENE_TYPE_DEVELOPMENT, gc) {}
	Gene(GeneLocalAttribute const &gla) : Gene(GENE_TYPE_PART_ATTRIBUTE, gla) {}
	Gene(GeneGeneralAttribute const &gga) : Gene(GENE_TYPE_GENERAL_ATTRIB, gga) {}
	Gene(GeneSynapse const &gs) : Gene(GENE_TYPE_SYNAPSE, gs) {}
	Gene(GeneFeedbackSynapse const &gfs) : Gene(GENE_TYPE_FEEDBACK_SYNAPSE, gfs) {}
	Gene(GeneTransferFunction const &gt) : Gene(GENE_TYPE_TRANSFER, gt) {}
	Gene(GeneNeuralConstant const &gnc) : Gene(GENE_TYPE_NEURAL_CONST, gnc) {}
	Gene(GeneBodyAttribute const &gba) : Gene(GENE_TYPE_BODY_ATTRIBUTE, gba) {}

	Gene(const Gene& original)
		: type(original.type)
		, data(original.data)
		, chance_to_delete(original.chance_to_delete)
		, chance_to_swap(original.chance_to_swap)
		, mutation_reference_value(original.mutation_reference_value)
	{
		update_meta_genes_vec();
	}

	std::vector<MetaGene*> metaGenes;

	MetaGene chance_to_delete;		// [0..1] represents the likelihood that this gene will disappear completely
	MetaGene chance_to_swap;		// likelihood that this gene will swap places with an adjacent one

	MetaGene mutation_reference_value; // maximum value by which a gene can be mutated. The mutation is random between - and + this value

private:
	void update_meta_genes_vec();
};

#endif //__gene_h__
