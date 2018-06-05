/*
 *	a gene is the fundamental genetic unit of information. It cannot be subdivided
 *	Genes may be altered by mutation but they are always inherited as a whole.
 */

#ifndef __gene_h__
#define __gene_h__

#include "constants.h"
#include "GeneDefinitions.h"

#include <boglfw/utils/rand.h>

#include <stdint.h>
#include <map>
#include <vector>
#include <cstring>

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

	Atom() = default;
	Atom(Atom const& a) = default;
};

constexpr bool fBool(Atom<float> const& f) { return f.value > 0; }

/*
 * This holds a sequence of values indicating the branch-and-depth based restrictions for another gene.
 * activeLevels holds the number of levels from levels[] that are to be used. Any level that is unused is considered by default least restrictive.
 * the value of activeLevels should be considered modulo MAX_DIVISION_DEPTH in order to avoid overflow
 *
 * * skip rules apply on current cell based on its handedness
 * * stop rules apply on the node's subtrees (left or right) - so if we have stopRight>0 at level 1, its level 2 right subtree will not get the gene
 */
struct BranchRestriction {
	struct levelRule {
		Atom<float> skipLeft;		// restricts application if >0 and current cell is left-handed
		Atom<float> skipRight;		// restricts application if >0 and current cell is right-handed
		Atom<float> stopLeft;		// if >0 blocks gene propagation down the left subtree
		Atom<float> stopRight;		// if >0 blocks gene propagation down the right subtree

		// for level 0, application is restricted if either skipLeft or skipRight is >0
	};
	levelRule levels[constants::MAX_DIVISION_DEPTH];	// default value for each level is most permissive due to strict >0 comparison above ^
	Atom<unsigned> activeLevels;

	void clear() {
		activeLevels.set(0u);
		memset(levels, 0, sizeof(levels));
	}

	BranchRestriction() {
		clear();
	}

	/* code contains pairs of characters separated by spaces, indicating the following:
	 * first char:
	 * 		'0' - apply to none (skip both)
	 * 		'L' - apply to left only (skip right)
	 * 		'R' - apply to right only (skip left)
	 * 		'*' - apply to both
	 * second char:
	 * 		'v' - propagate both ways
	 * 		'<' - propagate left only (stop right)
	 * 		'>' - propagate right only (stop left)
	 * 		'-' - stop both ways (don't propagate at all)
	 */
	BranchRestriction(const char* code) {
		clear();
		activeLevels.set((strlen(code)+1)/3);
		memset(levels, 0, sizeof(levels));
		for (uint i=0; i<activeLevels; i++) {
			switch (code[i*3+0]) {
				case '0':
					levels[i].skipLeft.set(constants::FBOOL_true);
					levels[i].skipRight.set(constants::FBOOL_true);
					break;
				case 'L':
					levels[i].skipRight.set(constants::FBOOL_true);
					break;
				case 'R':
					levels[i].skipLeft.set(constants::FBOOL_true);
					break;
				case '*':
					break;
				default:
					throw std::runtime_error("Unknown symbol in restriction code: " + code[i*3+0]);
			}
			switch (code[i*3+1]) {
			case '<':
				levels[i].stopRight.set(constants::FBOOL_true);
				break;
			case '>':
				levels[i].stopLeft.set(constants::FBOOL_true);
				break;
			case '-':
				levels[i].stopLeft.set(constants::FBOOL_true);
				levels[i].stopRight.set(constants::FBOOL_true);
				break;
			case 'v':
				break;
				default:
					throw std::runtime_error("Unknown symbol in restriction code: " + code[i*3+1]);
			}
		}
	}
};

struct GeneStop {
};

struct GeneNoOp {
};

struct GeneSkip {
	BranchRestriction restriction;
	Atom<int> count;

	GeneSkip() {
		restriction = BranchRestriction("*v");
		count.set(2);
	}
};

struct GeneDivisionParam {
	BranchRestriction restriction;
	gene_division_param_type param = GENE_DIVISION_INVALID;
	Atom<float> value;

	GeneDivisionParam() = default;
};

struct GeneJointAttribute {
	BranchRestriction restriction;
	gene_joint_attribute_type attrib = GENE_JOINT_ATTR_INVALID;
	Atom<float> value;

	GeneJointAttribute() = default;
};

struct GeneMuscleAttribute {
	BranchRestriction restriction;
	Atom<float> side;				// distinguish between the two muscles: negative is left, positive is right, zero is both
	gene_muscle_attribute_type attrib = GENE_MUSCLE_ATTR_INVALID;
	Atom<float> value;

	GeneMuscleAttribute() = default;
};

// this gene controls the genome offset (relative to the current part's) of the child cell in the given side
struct GeneOffset {
	BranchRestriction restriction;
	Atom<int> offset;
	Atom<float> side;	// negative is right, positive is left, 0 is both
};

struct GeneProtein {
	BranchRestriction restriction;
	gene_protein_type protein;		// the type of protein this gene produces
	Atom<float> weight;				// +/- moves the axis coordinate
};

struct GeneAttribute {
	BranchRestriction restriction;
	gene_part_attribute_type attribute = GENE_ATTRIB_INVALID;
	Atom<float> value;

	GeneAttribute() = default;
};

struct GeneVMSOffset {
	BranchRestriction restriction;
	Atom<float> value;
};

struct GeneNeuron {
	Atom<float> neuronLocation;		// neuron location in VMS
};

struct GeneTransferFunction {
	Atom<float> neuronLocation;		// neuron location
	Atom<int> functionID;
};

struct GeneNeuralBias {
	Atom<float> neuronLocation;		// neuron location
	Atom<float> value;
};

struct GeneNeuralParam {
	Atom<float> neuronLocation;		// neuron location
	Atom<float> value;
};

struct GeneSynapse {
	Atom<float> srcLocation;		// VMS coordinate of source (neuron output or sensor output)
	Atom<float> destLocation;		// VMS coordinate of destination (neuron input or motor input)
	Atom<float> weight;				// absolute weight of the synapse (cummulative)
	Atom<float> priority; 			// synapse priority - inputs synapses in a neuron are ordered by highest priority first
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
		GeneStop gene_stop;
		GeneNoOp gene_no_op;
		GeneSkip gene_skip;
		GeneDivisionParam gene_division_param;
		GeneProtein gene_protein;
		GeneOffset gene_offset;
		GeneAttribute gene_attribute;
		GeneJointAttribute gene_joint_attrib;
		GeneMuscleAttribute gene_muscle_attrib;
		GeneVMSOffset gene_vms_offset;
		GeneNeuron gene_neuron;
		GeneSynapse gene_synapse;
		GeneTransferFunction gene_transfer_function;
		GeneNeuralBias gene_neural_constant;
		GeneNeuralParam gene_neural_param;
		GeneBodyAttribute gene_body_attribute;

		GeneData(GeneStop const &gs) : gene_stop(gs) {}
		GeneData(GeneNoOp const &gnop) : gene_no_op(gnop) {}
		GeneData(GeneSkip const &gs) : gene_skip(gs) {}
		GeneData(GeneDivisionParam const &gdp) : gene_division_param(gdp) {}
		GeneData(GeneProtein const &gp) : gene_protein(gp) {}
		GeneData(GeneOffset const &go) : gene_offset(go) {}
		GeneData(GeneAttribute const &gla) : gene_attribute(gla) {}
		GeneData(GeneJointAttribute const &gja) : gene_joint_attrib(gja) {}
		GeneData(GeneMuscleAttribute const& gma) : gene_muscle_attrib(gma) {}
		GeneData(GeneVMSOffset const& gvo) : gene_vms_offset(gvo) {}
		GeneData(GeneNeuron const& gn) : gene_neuron(gn) {}
		GeneData(GeneSynapse const &gs) : gene_synapse(gs) {}
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

	Gene(GeneStop const &gs) : Gene(gene_type::STOP, gs) {}
	Gene(GeneNoOp const &gnop) : Gene(gene_type::NO_OP, gnop) {}
	Gene(GeneSkip const &gs) : Gene(gene_type::SKIP, gs) {}
	Gene(GeneDivisionParam const &gdp) : Gene(gene_type::DIVISION_PARAM, gdp) {}
	Gene(GeneProtein const &gp) : Gene(gene_type::PROTEIN, gp) {}
	Gene(GeneOffset const &go) : Gene(gene_type::OFFSET, go) {}
	Gene(GeneAttribute const &gla) : Gene(gene_type::PART_ATTRIBUTE, gla) {}
	Gene(GeneJointAttribute const &gja) : Gene(gene_type::JOINT_ATTRIBUTE, gja) {}
	Gene(GeneMuscleAttribute const &gma) : Gene(gene_type::MUSCLE_ATTRIBUTE, gma) {}
	Gene(GeneVMSOffset const& gvo) : Gene(gene_type::VMS_OFFSET, gvo) {}
	Gene(GeneNeuron const& gn) : Gene(gene_type::NEURON, gn) {}
	Gene(GeneSynapse const &gs) : Gene(gene_type::SYNAPSE, gs) {}
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
	static Gene createRandom(int spaceLeftAfter);

	std::vector<MetaGene*> metaGenes;

	MetaGene chance_to_delete;		// [0..1] represents the likelihood that this gene will disappear completely
	MetaGene chance_to_swap;		// likelihood that this gene will swap places with an adjacent one

private:
	void update_meta_genes_vec();

	static Gene createRandomSkipGene(int spaceLeftAfter);
	static Gene createRandomProteinGene();
	static Gene createRandomOffsetGene(int spaceLeftAfter);
	static Gene createRandomAttribGene();
	static Gene createRandomNeuronGene();
	static Gene createRandomSynapseGene();
	static Gene createRandomTransferFuncGene();
	static Gene createRandomNeuralBiasGene();
	static Gene createRandomNeuralParamGene();
	static Gene createRandomBodyAttribGene();
	static Gene createRandomDivisionParamGene();
	static Gene createRandomVMSOffsetGene();
	static Gene createRandomJointAttributeGene();
	static Gene createRandomMuscleAttributeGene();
};

#endif //__gene_h__
