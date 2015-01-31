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
	MetaGene chanceToMutate;	// chance that this atom will mutate
	MetaGene changeAmount;	// maximum value by which a gene can be mutated. The mutation is random between - and + this value

	operator T() const { return value; }
	void set(T value) {
		this->value = value;
		this->chanceToMutate.value = constants::initial_gene_mutate;
		this->chanceToMutate.dynamic_variation = constants::change_gene_mutate;
		this->changeAmount.value = constants::initial_gene_mutation_value;
		this->changeAmount.dynamic_variation = constants::change_gene_mutation_value;
	}

	Atom() : value(), chanceToMutate(), changeAmount() {}
};

struct GeneStop {
};

struct GeneCommand {
	gene_development_command command;
	gene_part_type part_type;
	Atom<float> angle;				// angle is relative to the previous element's orientation
	Atom<int> genomeOffset;			// offset from current gene to the start of the genes for the new part
	Atom<int> genomeOffsetJoint;	// offset from current gene to the start of the genes for the new part's joint
	Atom<int> genomeOffsetMuscle1;	// offset from current gene to the start of the genes for the new part's muscle 1
	Atom<int> genomeOffsetMuscle2;	// offset from current gene to the start of the genes for the new part's muscle 2
};

struct GeneAttribute {
	Atom<float> value;
	gene_part_attribute_type attribute;
	bool relativeValue;
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
		GeneStop gene_stop;
		GeneCommand gene_command;
		GeneAttribute gene_attribute;
		GeneSynapse gene_synapse;
		GeneTransferFunction gene_transfer_function;
		GeneNeuralConstant gene_neural_constant;
		GeneFeedbackSynapse gene_feedback_synapse;
		GeneBodyAttribute gene_body_attribute;

		GeneData(GeneStop const &gs) : gene_stop(gs) {}
		GeneData(GeneCommand const &gc) : gene_command(gc) {}
		GeneData(GeneAttribute const &gla) : gene_attribute(gla) {}
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
	{
		update_meta_genes_vec();
	}

	Gene(GeneStop const& gs) : Gene(GENE_TYPE_STOP, gs) {}
	Gene(GeneCommand const &gc) : Gene(GENE_TYPE_DEVELOPMENT, gc) {}
	Gene(GeneAttribute const &gla) : Gene(GENE_TYPE_PART_ATTRIBUTE, gla) {}
	Gene(GeneSynapse const &gs) : Gene(GENE_TYPE_SYNAPSE, gs) {}
	Gene(GeneFeedbackSynapse const &gfs) : Gene(GENE_TYPE_FEEDBACK_SYNAPSE, gfs) {}
	Gene(GeneTransferFunction const &gt) : Gene(GENE_TYPE_TRANSFER, gt) {}
	Gene(GeneNeuralConstant const &gnc) : Gene(GENE_TYPE_NEURAL_CONST, gnc) {}
	Gene(GeneBodyAttribute const &gba) : Gene(GENE_TYPE_BODY_ATTRIBUTE, gba) {}

	Gene(const Gene& original)
		: RID(original.RID)
		, type(original.type)
		, data(original.data)
		, chance_to_delete(original.chance_to_delete)
		, chance_to_swap(original.chance_to_swap)
	{
		update_meta_genes_vec();
	}

	Gene& operator=(Gene const& right) {
		RID = right.RID;
		type = right.type;
		data = right.data;
		chance_to_delete = right.chance_to_delete;
		chance_to_swap = right.chance_to_swap;
		update_meta_genes_vec();
		return *this;
	}

	static Gene createRandom();

	std::vector<MetaGene*> metaGenes;

	MetaGene chance_to_delete;		// [0..1] represents the likelihood that this gene will disappear completely
	MetaGene chance_to_swap;		// likelihood that this gene will swap places with an adjacent one

private:
	void update_meta_genes_vec();
};

#endif //__gene_h__
