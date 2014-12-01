/*
 *	a ribosome decodes a sequence of chromosomes and builds a fully-functional neural network
 */

#ifndef __ribosome_h__
#define __ribosome_h__

#include "Chromosome.h"

class Chromosome;
class Bug;

class Ribosome {
public:
	// recombines genes from the two parent Genomes and creates an offspring Genome
	// the resulting offspring may have some genes and/or chromosomes altered by mutation
	static Genome recombine_offspring(Genome &Genome1, Genome &Genome2, int requiredOutputs, int availableInputs);

	// create a new chromosome filled with random genes
	// parameters define maximum number of default (network-level) inputs
	// and
	// maximum number of non-default inputs (synapse input from other neurons)
	// for the neuron described by the new chromosome
	static Chromosome createRandomChromosome(unsigned maxInputs, unsigned maxDefaultInputs);

	// creates a random Genome; maxChromosomes must be greater than nDefaultInputs and requiredOutputs
	static Genome createRandomGenome(unsigned maxChromosomes, unsigned nDefaultInputs, unsigned requiredOutputs);

	// decodes an embryonic network's Genome and develops that network according to the Genome
	// into a fully functional network
	static void decode_and_develop_entity(Bug* in_out_bug);
};

#endif //__ribosome_h__
