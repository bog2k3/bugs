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
	float value;
	float dynamic_variation;

	MetaGene(float initial_reference_value, float dynamic_variation)
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

	Atom(Atom const& a) = default;
};

struct GeneStartMarker {
};

struct GeneStop {
};

struct GeneNoOp {
};

struct GeneSkip {
	Atom<int> minDepth;
	Atom<int> maxDepth;
	Atom<int> count;

	GeneSkip() {
		minDepth.set(1);
		maxDepth.set(1);
		count.set(2);
	}
};

// this gene controls the genome offset (relative to the current part's) of the child spawned from a given target segment
struct GeneOffset {
	Atom<int> minDepth;
	Atom<int> maxDepth;
	Atom<int> offset;
	Atom<int> targetSegment;
};

// this gene controls the genome offset (relative to the current part's) of the upstream Joint of this part, if it exists
struct GeneJointOffset {
	Atom<int> minDepth;
	Atom<int> maxDepth;
	Atom<int> offset;
};

struct GeneProtein {
	Atom<gene_protein_type> protein;				// the type of protein this gene produces
	Atom<int> targetSegment;						// target segment of current part which protein affects
	Atom<int> minDepth;								// min hierarchical level where gene activates
	Atom<int> maxDepth;								// max hierarchical level where gene activates
};

struct GeneAttribute {
	Atom<float> value;
	Atom<int> minDepth;
	Atom<int> maxDepth;
	Atom<int> attribIndex;							// some attributes are indexed (like VMS coords for inputs/outputs)
	gene_part_attribute_type attribute = GENE_ATTRIB_INVALID;

	GeneAttribute() = default;
};

struct GeneSynapse {
	Atom<int> from;		// virtual neuron index
	Atom<int> to;		// virtual neuron index
	Atom<float> weight;	// absolute weight of the synapse (cummulative)
	Atom<float> priority; // synapse priority - inputs synapses in a neuron are ordered by highest priority first
};

struct GeneNeuronOutputCoord {
	Atom<int> srcNeuronVirtIndex;		// virtual index of neuron in neural network that will output to a motor
	Atom<float> outCoord;				// coordinate in motor virtual matching space
};

struct GeneNeuronInputCoord {
	Atom<int> destNeuronVirtIndex;		// virtual index of neuron in neural network that will receive the input from sensors
	Atom<float> inCoord;				// coordinate in sensor virtual matching space
};

struct GeneTransferFunction {
	Atom<int> targetNeuron;
	Atom<int> functionID;
};

struct GeneNeuralBias {
	Atom<int> targetNeuron;
	Atom<float> value;
};

struct GeneNeuralParam {
	Atom<int> targetNeuron;
	Atom<float> value;
};

struct GeneBodyAttribute {
	gene_body_attribute_type attribute = GENE_BODY_ATTRIB_INVALID;
	Atom<float> value;

	GeneBodyAttribute() = default;
};

class Gene {
public:
	gene_type type;		// the type of gene
	union GeneData {
		GeneStartMarker gene_start_marker;
		GeneStop gene_stop;
		GeneNoOp gene_no_op;
		GeneSkip gene_skip;
		GeneProtein gene_protein;
		GeneOffset gene_offset;
		GeneJointOffset gene_joint_offset;
		GeneAttribute gene_attribute;
		GeneSynapse gene_synapse;
		GeneNeuronOutputCoord gene_neuron_output;
		GeneNeuronInputCoord gene_neuron_input;
		GeneTransferFunction gene_transfer_function;
		GeneNeuralBias gene_neural_constant;
		GeneNeuralParam gene_neural_param;
		GeneBodyAttribute gene_body_attribute;

		GeneData(GeneStartMarker const& gsm) : gene_start_marker(gsm) {}
		GeneData(GeneStop const &gs) : gene_stop(gs) {}
		GeneData(GeneNoOp const &gnop) : gene_no_op(gnop) {}
		GeneData(GeneSkip const &gs) : gene_skip(gs) {}
		GeneData(GeneProtein const &gp) : gene_protein(gp) {}
		GeneData(GeneOffset const &go) : gene_offset(go) {}
		GeneData(GeneJointOffset const& gjo) : gene_joint_offset(gjo) {}
		GeneData(GeneAttribute const &gla) : gene_attribute(gla) {}
		GeneData(GeneSynapse const &gs) : gene_synapse(gs) {}
		GeneData(GeneNeuronOutputCoord const &gno) : gene_neuron_output(gno) {}
		GeneData(GeneNeuronInputCoord const& gni) : gene_neuron_input(gni) {}
		GeneData(GeneTransferFunction const &gt) : gene_transfer_function(gt) {}
		GeneData(GeneNeuralBias const &gnc) : gene_neural_constant(gnc) {}
		GeneData(GeneNeuralParam const& gnp) : gene_neural_param(gnp) {}
		GeneData(GeneBodyAttribute const &gba) : gene_body_attribute(gba) {}
	} data;

	Gene(gene_type type, GeneData data)
		: type(type)
		, data(data)
		, chance_to_delete(constants::initial_gene_delete, constants::change_gene_delete)
		, chance_to_swap(constants::initial_gene_swap, constants::change_gene_swap)
	{
		update_meta_genes_vec();
	}

#ifdef ENABLE_START_MARKER_GENES
	Gene(GeneStartMarker const& gsm) : Gene(gene_type::START_MARKER, gsm) {}
#endif
	Gene(GeneStop const &gs) : Gene(gene_type::STOP, gs) {}
	Gene(GeneNoOp const &gnop) : Gene(gene_type::NO_OP, gnop) {}
	Gene(GeneSkip const &gs) : Gene(gene_type::SKIP, gs) {}
	Gene(GeneProtein const &gp) : Gene(gene_type::PROTEIN, gp) {}
	Gene(GeneOffset const &go) : Gene(gene_type::OFFSET, go) {}
	Gene(GeneJointOffset const& gjo) : Gene(gene_type::JOINT_OFFSET, gjo) {}
	Gene(GeneAttribute const &gla) : Gene(gene_type::PART_ATTRIBUTE, gla) {}
	Gene(GeneSynapse const &gs) : Gene(gene_type::SYNAPSE, gs) {}
	Gene(GeneNeuronOutputCoord const &gnoc) : Gene(gene_type::NEURON_OUTPUT_COORD, gnoc) {}
	Gene(GeneNeuronInputCoord const& gnic) : Gene(gene_type::NEURON_INPUT_COORD, gnic) {}
	Gene(GeneTransferFunction const &gt) : Gene(gene_type::TRANSFER_FUNC, gt) {}
	Gene(GeneNeuralBias const &gnc) : Gene(gene_type::NEURAL_BIAS, gnc) {}
	Gene(GeneNeuralParam const& gnp) : Gene(gene_type::NEURAL_PARAM, gnp) {}
	Gene(GeneBodyAttribute const &gba) : Gene(gene_type::BODY_ATTRIBUTE, gba) {}

	Gene() : Gene(GeneNoOp()) {}

	Gene(const Gene& original)
		: type(original.type)
		, data(original.data)
		, chance_to_delete(original.chance_to_delete)
		, chance_to_swap(original.chance_to_swap)
	{
		update_meta_genes_vec();
	}

	Gene& operator=(Gene const& right) {
		type = right.type;
		data = right.data;
		chance_to_delete = right.chance_to_delete;
		chance_to_swap = right.chance_to_swap;
		update_meta_genes_vec();
		return *this;
	}

	char getSymbol() const;

	// @spaceLeftAfter tells how many genes are in the chromosome after the position where this one will be inserted
	// @nNeurons tells how many neurons the genome creates
	static Gene createRandom(int spaceLeftAfter, int nNeurons);

	std::vector<MetaGene*> metaGenes;

	MetaGene chance_to_delete;		// [0..1] represents the likelihood that this gene will disappear completely
	MetaGene chance_to_swap;		// likelihood that this gene will swap places with an adjacent one

private:
	void update_meta_genes_vec();

	static Gene createRandomSkipGene(int spaceLeftAfter);
	static Gene createRandomProteinGene();
	static Gene createRandomOffsetGene(int spaceLeftAfter);
	static Gene createRandomJointOffsetGene(int spaceLeftAfter);
	static Gene createRandomAttribGene();
	static Gene createRandomSynapseGene(int nNeurons);
	static Gene createRandomNeuronInputCoordGene(int nNeurons);
	static Gene createRandomNeuronOutputCoordGene(int nNeurons);
	static Gene createRandomTransferFuncGene(int nNeurons);
	static Gene createRandomNeuralBiasGene(int nNeurons);
	static Gene createRandomNeuralParamGene(int nNeurons);
	static Gene createRandomBodyAttribGene();
};

#endif //__gene_h__
