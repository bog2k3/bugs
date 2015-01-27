/*
 * Genome.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#include "Genome.h"
#include "Gene.h"
#include "../utils/rand.h"
#include "../utils/log.h"
#include "../math/math2D.h"

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
	static constexpr float numberGeneTypes = 9;
	static constexpr float averageAtomsPerGene = (float)(constants::MAX_GROWTH_DEPTH + 14) / numberGeneTypes;
	static constexpr float numberAlterationsPer100Atoms = 5;	// how many alterations we desire for a given number of atoms in a chromosome

	// compute the total number of changes desired for this chromosome:
	// (this includes swapping and deletion, but NOT creating new genes - that will be handled separately later)
	float totalChangesDesired = c.size() * averageAtomsPerGene * 1.e-2 * numberAlterationsPer100Atoms;
	// compute the total chance for mutations in the current chromosome:
	float totalChanceToChange = 0.f;
	for (unsigned i=0; i<c.size(); i++) {
		totalChanceToChange += getTotalMutationChance(c[i]);
	}
	// now we compute a factor to multiply the mutation chances to bring them into the desired range
	float mutationChanceFactor = totalChangesDesired / totalChanceToChange;

	// now we go ahead with mutations:
	for (unsigned i=0; i<c.size(); i++) {
		bool delGene = randf() < c[i].chance_to_delete.value * mutationChanceFactor;
		if (delGene) {
			c.erase(c.begin()+i);
			i--;
			continue;
		}
		bool swap = randf() < c[i].chance_to_swap.value * mutationChanceFactor;
		bool swapReverse = false;
		if (swap) {
			if (i < c.size()-1) // swap ahead
				xchg(c[i], c[i+1]);
			else if (i > 0) { // swap behind
				xchg(c[i], c[i-1]);
				swapReverse = true;
			}
		}
		if (swapReverse)
			alterGene(c[i-1], mutationChanceFactor);
		else
			alterGene(c[i], mutationChanceFactor);

		if (swap && !swapReverse) {
			// swapped gene has been altered partially (by swapping), so must not go through a complete step again, do the rest here:
			alterGene(c[i+1], mutationChanceFactor);
			i++; // skip the next gene that has already been altered
		}
	}

	// now there's a chance to spawn a new gene
	if (randf() < constants::global_chance_to_spawn_gene) {
		c.push_back(Gene::createRandom());
	}
}

template<typename T>
void alterAtom(Atom<T> &a, float mutationChanceFactor) {
	float chance = a.chanceToMutate.value * mutationChanceFactor;
	if (chance < constants::global_alteration_override_chance)
		chance = constants::global_alteration_override_chance;
	if (randf() < chance) {
		if (std::is_integral<T>::value) {
			a.value += (randf()<0.5f) ? +1 : -1;
		} else {
			a.value += srandf() * a.changeAmount.value;
		}
	}
}

void GeneticOperations::alterGene(Gene &g, float mutationChanceFactor) {
	switch (g.type) {
	case GENE_TYPE_BODY_ATTRIBUTE:
		alterAtom(g.data.gene_body_attribute.value, mutationChanceFactor);
		break;
	case GENE_TYPE_DEVELOPMENT:
		alterAtom(g.data.gene_command.angle, mutationChanceFactor);
		break;
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		alterAtom(g.data.gene_feedback_synapse.from, mutationChanceFactor);
		alterAtom(g.data.gene_feedback_synapse.to, mutationChanceFactor);
		alterAtom(g.data.gene_feedback_synapse.weight, mutationChanceFactor);
		break;
	case GENE_TYPE_GENERAL_ATTRIB:
		alterAtom(g.data.gene_general_attribute.value, mutationChanceFactor);
		break;
	case GENE_TYPE_LOCATION:
		for (unsigned i=0; i<constants::MAX_GROWTH_DEPTH; i++)
			alterAtom(g.data.gene_location.location[i], mutationChanceFactor);
		break;
	case GENE_TYPE_NEURAL_CONST:
		alterAtom(g.data.gene_neural_constant.targetNeuron, mutationChanceFactor);
		alterAtom(g.data.gene_neural_constant.value, mutationChanceFactor);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		alterAtom(g.data.gene_local_attribute.value, mutationChanceFactor);
		break;
	case GENE_TYPE_SYNAPSE:
		alterAtom(g.data.gene_synapse.from, mutationChanceFactor);
		alterAtom(g.data.gene_synapse.to, mutationChanceFactor);
		alterAtom(g.data.gene_synapse.weight, mutationChanceFactor);
		break;
	case GENE_TYPE_TRANSFER:
		alterAtom(g.data.gene_transfer_function.functionID, mutationChanceFactor);
		alterAtom(g.data.gene_transfer_function.targetNeuron, mutationChanceFactor);
		break;
	default:
		ERROR("unhandled gene type: "<<g.type);
		break;
	}

	for (auto m : g.metaGenes)
		alterMetaGene(*m);
}

float GeneticOperations::getTotalMutationChance(Gene const& g) {
	float ret = g.chance_to_delete.value + g.chance_to_swap.value;
	switch (g.type) {
	case GENE_TYPE_BODY_ATTRIBUTE:
		ret += g.data.gene_body_attribute.value.chanceToMutate.value;
		break;
	case GENE_TYPE_DEVELOPMENT:
		ret += g.data.gene_command.angle.chanceToMutate.value;
		break;
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		ret += g.data.gene_feedback_synapse.from.chanceToMutate.value;
		ret += g.data.gene_feedback_synapse.to.chanceToMutate.value;
		ret += g.data.gene_feedback_synapse.weight.chanceToMutate.value;
		break;
	case GENE_TYPE_GENERAL_ATTRIB:
		ret += g.data.gene_general_attribute.value.chanceToMutate.value;
		break;
	case GENE_TYPE_LOCATION:
		for (unsigned i=0; i<constants::MAX_GROWTH_DEPTH; i++)
			ret += g.data.gene_location.location[i].chanceToMutate.value;
		break;
	case GENE_TYPE_NEURAL_CONST:
		ret += g.data.gene_neural_constant.value.chanceToMutate.value;
		ret += g.data.gene_neural_constant.targetNeuron.chanceToMutate.value;
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		ret += g.data.gene_local_attribute.value.chanceToMutate.value;
		break;
	case GENE_TYPE_SYNAPSE:
		ret += g.data.gene_synapse.from.chanceToMutate.value;
		ret += g.data.gene_synapse.to.chanceToMutate.value;
		ret += g.data.gene_synapse.weight.chanceToMutate.value;
		break;
	case GENE_TYPE_TRANSFER:
		ret += g.data.gene_transfer_function.functionID.chanceToMutate.value;
		ret += g.data.gene_transfer_function.targetNeuron.chanceToMutate.value;
		break;
	default:
		ERROR("unhandled gene type: "<<g.type);
		break;
	}
	return ret;
}


void GeneticOperations::alterMetaGene(MetaGene &meta)
{
	meta.value += srandf() * meta.dynamic_variation;
	if (meta.value < 0)
		meta.value = 0;
}

