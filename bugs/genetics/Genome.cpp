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
	while (i<gen.first.genes.size() || i<gen.second.genes.size()) {
		const Gene *g = nullptr;
		if (i<gen.first.genes.size()) {
			g = &gen.first.genes[i];
			if (i<gen.second.genes.size() && randf() < 0.5f)
				g = &gen.second.genes[i];
		} else
			g = &gen.second.genes[i];
		c.genes.push_back(*g);
		if (g->type == GENE_TYPE_DEVELOPMENT)
			c.genes[i].data.gene_command.age++;	// should probably rebase all to 0 to avoid unsigned overflow - unlikely
		i++;
	}
#error must also copy the insertion positions
	alterChromosome(c);
	return c;
}

void GeneticOperations::fixGenesSynchro(Genome& gen) {
	// this shit is more complicated than i thought
	assert(abs(gen.first.genes.size() - gen.second.genes.size()) <= WorldConst::MaxGenomeLengthDifference);
	int dif = gen.first.genes.size() - gen.second.genes.size();
	// pull back the insertions on the once chromosome that is shorter:
	if (dif > 0) {
		// C2 is shorter
		for (int i=0; i<WorldConst::MaxGenomeLengthDifference; i++) {
#warning must also compute the number of additions from each chromosome - their difference must be equal to chrom length diff, otherwise pull back one of them.
			int from = i + dif
		}
	} else if (dif < 0) {
		// C1 is shorter
	}
	// now the difference in number of insertions is equal to the difference in chromosome length, we can assume pairs of simultaneous insertions
	// start from the oldest addition from each chromosome, and add it into the other (if they're on different positions), computing
	// the offseted insertion position by looking at other insertions if either of them occures before or on the same pos.
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

	int nMotors = 0, nSensors = 0;
	std::map<int, bool> mapNeuronsExist;

	static constexpr float numberAlterationsPerChromosome = 3;	// how many alterations we desire for a chromosome at most

	// compute the total chance for mutations in the current chromosome:
	// also count the number of neurons, motors and sensors since we need these in order to create new random genes
	float totalChanceToChange = 0.f;
	for (unsigned i=0; i<c.genes.size(); i++) {
		totalChanceToChange += getTotalMutationChance(c.genes[i]);

		// count
		switch (c.genes[i].type) {
		case GENE_TYPE_DEVELOPMENT:
			if (c.genes[i].data.gene_command.command == GENE_DEV_SPLIT) {
				nMotors += 5;
				nSensors += 5;
				break;
			}
			switch (c.genes[i].data.gene_command.part_type) {
			case GENE_PART_BONE:
				nMotors += 2;
				nSensors++;
				break;
			case GENE_PART_EGGLAYER:
				nMotors += 2;
				nSensors++;
				break;
			case GENE_PART_GRIPPER:
				nMotors++;
				break;
			case GENE_PART_SENSOR:
				nSensors++;
				break;
			default:
				break;
			}
			break;
		case GENE_TYPE_FEEDBACK_SYNAPSE:
			if (c.genes[i].data.gene_feedback_synapse.to >= 0)
				mapNeuronsExist[c.genes[i].data.gene_feedback_synapse.to] = true;
			break;
		case GENE_TYPE_SYNAPSE:
			if (c.genes[i].data.gene_synapse.from >= 0)
				mapNeuronsExist[c.genes[i].data.gene_synapse.from] = true;
			if (c.genes[i].data.gene_synapse.to >= 0)
				mapNeuronsExist[c.genes[i].data.gene_synapse.to] = true;
			break;
		default:
			break;
		}
	}
	int nNeurons = mapNeuronsExist.size();

	// now we compute a factor to multiply the mutation chances to bring them into the desired range
	float mutationChanceFactor = totalChanceToChange;
	if (mutationChanceFactor * c.genes.size() > numberAlterationsPerChromosome)
		mutationChanceFactor = numberAlterationsPerChromosome / mutationChanceFactor;

	// now we go ahead with mutations:
	for (unsigned i=0; i<c.genes.size(); i++) {
		bool delGene = randf() < c.genes[i].chance_to_delete.value * mutationChanceFactor;
		if (delGene) {
			c.genes[i].type = GENE_TYPE_NO_OP;
#ifdef DEBUG
			stat_delete++;
#endif
			continue;
		}
		bool swap = randf() < c.genes[i].chance_to_swap.value * mutationChanceFactor;
		bool swapReverse = false;
		if (swap) {
			if (i < c.genes.size()-1) { // swap ahead
				xchg(c.genes[i], c.genes[i+1]);
#ifdef DEBUG
				stat_swaps++;
#endif
			} else if (i > 0) { // swap behind
				xchg(c.genes[i], c.genes[i-1]);
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
			alterGene(c.genes[i-1], mutationChanceFactor);
		} else {
#ifdef DEBUG
			stat_mutations +=
#endif
			alterGene(c.genes[i], mutationChanceFactor);
		}

		if (swap && !swapReverse) {
			// swapped gene has been altered partially (by swapping), so must not go through a complete step again, do the rest here:
#ifdef DEBUG
			stat_mutations +=
#endif
			alterGene(c.genes[i+1], mutationChanceFactor);
			i++; // skip the next gene that has already been altered
		}
	}

	// now there's a chance to spawn a new gene
	if (randf() < constants::global_chance_to_spawn_gene * c.genes.size()) {
		int position = randi(c.genes.size()-1);
		Gene newGene(Gene::createRandom(c.genes.size()-position, nMotors, nSensors, nNeurons));
		if (c.genes[position].type == GENE_TYPE_NO_OP)
			c.genes[position] = newGene;
		else {
			// must keep a record of last genes inserted (at most N, and if gametes have a difference of more than N genes, they don't fuse)
			// when combining two gametes we must insert dummy genes at corespondend positions in the other chromosome, in order to realign the alelles.
			c.genes.insert(c.genes.begin()+position, newGene);
			for (int i=1; i<WorldConst::MaxGenomeLengthDifference; i++)
				c.lastInsertPos[i] = c.lastInsertPos[i-1];
			c.lastInsertPos[0] = position;
		}
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

