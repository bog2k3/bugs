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
#ifdef DEBUG
	int stat_mutations = 0;
	int stat_swaps = 0;
	int stat_delete = 0;
	int stat_new = 0;
#endif

	static constexpr float numberAlterationsPerChromosome = 1;	// how many alterations we desire for a chromosome

	// compute the total chance for mutations in the current chromosome:
	float totalChanceToChange = 0.f;
	for (unsigned i=0; i<c.size(); i++) {
		totalChanceToChange += getTotalMutationChance(c[i]);
	}
	// now we compute a factor to multiply the mutation chances to bring them into the desired range
	float mutationChanceFactor = numberAlterationsPerChromosome / totalChanceToChange;

	// now we go ahead with mutations:
	for (unsigned i=0; i<c.size(); i++) {
		bool delGene = randf() < c[i].chance_to_delete.value * mutationChanceFactor;
		if (delGene) {
			c.erase(c.begin()+i);
			i--;
#ifdef DEBUG
			stat_delete++;
#endif
			continue;
		}
		bool swap = randf() < c[i].chance_to_swap.value * mutationChanceFactor;
		bool swapReverse = false;
		if (swap) {
			if (i < c.size()-1) { // swap ahead
				xchg(c[i], c[i+1]);
#ifdef DEBUG
				stat_swaps++;
#endif
			} else if (i > 0) { // swap behind
				xchg(c[i], c[i-1]);
				swapReverse = true;
#ifdef DEBUG
				stat_swaps++;
#endif
			}
		}
		if (swapReverse) {
#ifdef DEBUG
			stat_mutations +=
#endif
			alterGene(c[i-1], mutationChanceFactor);
		} else {
#ifdef DEBUG
			stat_mutations +=
#endif
			alterGene(c[i], mutationChanceFactor);
		}

		if (swap && !swapReverse) {
			// swapped gene has been altered partially (by swapping), so must not go through a complete step again, do the rest here:
#ifdef DEBUG
			stat_mutations +=
#endif
			alterGene(c[i+1], mutationChanceFactor);
			i++; // skip the next gene that has already been altered
		}
	}

	// now there's a chance to spawn a new gene
	if (randf() < constants::global_chance_to_spawn_gene * c.size()) {
		c.insert(c.begin()+randf()*c.size(), Gene::createRandom());
#ifdef DEBUG
		stat_new++;
#endif
	}

#ifdef DEBUG
		LOGLN("alter chromosome: [mutations: "<<stat_mutations<<"] [swaps: "<<stat_swaps<<"] [new: "<<stat_new<<"] [del: "<<stat_delete<<"]");
#endif
}

template<typename T>
bool alterAtom(Atom<T> &a, float mutationChanceFactor) {
	float chance = a.chanceToMutate.value * mutationChanceFactor;
	if (chance < constants::global_alteration_override_chance)
		chance = constants::global_alteration_override_chance;
	if (randf() < chance) {
		if (std::is_integral<T>::value) {
			a.value += (randf()<0.5f) ? +1 : -1;
		} else {
			a.value += srandf() * a.changeAmount.value;
		}
		return true;
	} else
		return false;
}

int GeneticOperations::alterGene(Gene &g, float mutationChanceFactor) {
	int altered = 0;
	switch (g.type) {
	case GENE_TYPE_STOP:
		break;
	case GENE_TYPE_BODY_ATTRIBUTE:
		altered += alterAtom(g.data.gene_body_attribute.value, mutationChanceFactor);
		break;
	case GENE_TYPE_DEVELOPMENT:
		altered += alterAtom(g.data.gene_command.angle, mutationChanceFactor);
		altered += alterAtom(g.data.gene_command.genomeOffset, mutationChanceFactor);
		altered += alterAtom(g.data.gene_command.genomeOffsetMuscle1, mutationChanceFactor);
		altered += alterAtom(g.data.gene_command.genomeOffsetMuscle2, mutationChanceFactor);
		break;
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		altered += alterAtom(g.data.gene_feedback_synapse.from, mutationChanceFactor);
		altered += alterAtom(g.data.gene_feedback_synapse.to, mutationChanceFactor);
		altered += alterAtom(g.data.gene_feedback_synapse.weight, mutationChanceFactor);
		break;
	case GENE_TYPE_NEURAL_CONST:
		altered += alterAtom(g.data.gene_neural_constant.targetNeuron, mutationChanceFactor);
		altered += alterAtom(g.data.gene_neural_constant.value, mutationChanceFactor);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		altered += alterAtom(g.data.gene_attribute.value, mutationChanceFactor);
		break;
	case GENE_TYPE_SYNAPSE:
		altered += alterAtom(g.data.gene_synapse.from, mutationChanceFactor);
		altered += alterAtom(g.data.gene_synapse.to, mutationChanceFactor);
		altered += alterAtom(g.data.gene_synapse.weight, mutationChanceFactor);
		break;
	case GENE_TYPE_TRANSFER:
		altered += alterAtom(g.data.gene_transfer_function.functionID, mutationChanceFactor);
		altered += alterAtom(g.data.gene_transfer_function.targetNeuron, mutationChanceFactor);
		break;
	default:
		ERROR("unhandled gene type: "<<g.type);
		break;
	}

	// if any part of the gene has changed, then it becomes a new gene:
	if (altered)
		g.RID = new_RID();

	for (auto m : g.metaGenes) {
		alterMetaGene(*m);
	}

	return altered;
}

float GeneticOperations::getTotalMutationChance(Gene const& g) {
	float ret = g.chance_to_delete.value + g.chance_to_swap.value;
	switch (g.type) {
	case GENE_TYPE_STOP:
		break;
	case GENE_TYPE_BODY_ATTRIBUTE:
		ret += g.data.gene_body_attribute.value.chanceToMutate.value;
		break;
	case GENE_TYPE_DEVELOPMENT:
		ret += g.data.gene_command.angle.chanceToMutate.value;
		ret += g.data.gene_command.genomeOffset.chanceToMutate.value;
		ret += g.data.gene_command.genomeOffsetJoint.chanceToMutate.value;
		ret += g.data.gene_command.genomeOffsetMuscle1.chanceToMutate.value;
		ret += g.data.gene_command.genomeOffsetMuscle2.chanceToMutate.value;
		break;
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		ret += g.data.gene_feedback_synapse.from.chanceToMutate.value;
		ret += g.data.gene_feedback_synapse.to.chanceToMutate.value;
		ret += g.data.gene_feedback_synapse.weight.chanceToMutate.value;
		break;
	case GENE_TYPE_NEURAL_CONST:
		ret += g.data.gene_neural_constant.value.chanceToMutate.value;
		ret += g.data.gene_neural_constant.targetNeuron.chanceToMutate.value;
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		ret += g.data.gene_attribute.value.chanceToMutate.value;
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

