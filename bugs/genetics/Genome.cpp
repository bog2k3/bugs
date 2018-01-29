/*
 * Genome.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#include "Genome.h"

#include <boglfw/utils/rand.h>
#include <boglfw/utils/log.h>
#include <boglfw/math/math3D.h>

#include <set>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

std::string Chromosome::stringify() const {
	char s[genes.size() + (genes.size()/10)*4 + 1];
	uint p = 0;
	for (uint i=0; i<genes.size(); i++) {
		if (i % 10 == 0) {
			s[p++] = '[';
			s[p++] = ((i/100) % 10) + '0';
			s[p++] = ((i/10) % 10) + '0';
			s[p++] = ']';
		}
		s[p++] = genes[i].getSymbol();
	}
	s[p] = 0;
	return s;
}

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
		/*if (g->type == gene_type::DEVELOPMENT)
			c.genes[i].data.gene_command.age++;	// should probably rebase all to 0 to avoid unsigned overflow - unlikely
			*/
		i++;
	}
	// copy insertion list:
	c.insertions = gen.first.insertions;	// both chromosomes should have identical lists
	// increase all insertions' age:
	for (uint i=0; i<c.insertions.size(); i++)
		c.insertions[i].age++;
	// perform some mutations:
	alterChromosome(c);
	trimInsertionList(c);
	return c;
}

void GeneticOperations::pullBackInsertions(Chromosome &c, int amount) {
	assertDbg(amount > 0);
	for (uint i=0; i<c.insertions.size(); i++) {
		uint from = i + amount;
		if (from < c.insertions.size())
			c.insertions[i] = c.insertions[from];
		else {
			c.insertions.erase(c.insertions.begin()+i, c.insertions.end());
			break;
		}
	}
}

/*
 * this will insert a new gene and return the index in the insertions vector where this change has been recorded
 */
int GeneticOperations::insertNewGene(Chromosome &c, Chromosome::insertion ins, Gene const& g) {
	assertDbg(ins.index <= (int)c.genes.size());
	c.genes.insert(c.genes.begin() + ins.index, g);
	// determine where in insertions we must add this new index
	uint d=0;
	while (d<c.insertions.size() && c.insertions[d].index < ins.index) d++;
	c.insertions.insert(c.insertions.begin()+d, ins);
	int ret = d;
	// increment all insertions that are to the right of this one
	for (++d; d<c.insertions.size(); d++)
		c.insertions[d].index++;
	return ret;
}

void GeneticOperations::trimInsertionList(Chromosome &c) {
	while (c.insertions.size() > WorldConst::MaxGenomeLengthDifference) {
		// search for oldest entry and remove it
		uint ioldest = 0;
		for (uint i=1; i<c.insertions.size(); i++)
			if (c.insertions[i].age > c.insertions[ioldest].age)
				ioldest = i;
		c.insertions.erase(c.insertions.begin()+ioldest);
	}
}

void GeneticOperations::fixGenesSynchro(Genome& gen) {
	// this shit is more complicated than i thought
	LOGLN("chromosome diff: "<< (int)abs(gen.first.genes.size() - (int)gen.second.genes.size()));
	assertDbg((uint)abs((int)gen.first.genes.size() - (int)gen.second.genes.size()) <= WorldConst::MaxGenomeLengthDifference);

	// assumption: insertions list from each chromosome should be sorted from left to right (smallest index first)
	Chromosome &c1 = gen.first;
	Chromosome &c2 = gen.second;

	// compute the length difference between chromosomes:
	int dif = c1.genes.size() - c2.genes.size();
	// pull back the insertions on the one chromosome that is shorter until the difference in size matches the difference
	// in number of insertions:
	if (dif > 0) {
		assertDbg(c1.insertions.size() >= c2.insertions.size());
		// C2 is shorter
		int pullback = dif - (c1.insertions.size() - c2.insertions.size());
		if (pullback > 0)
			pullBackInsertions(c2, pullback);
	} else if (dif < 0) {
		// C1 is shorter
		assertDbg(c1.insertions.size() <= c2.insertions.size());
		int pullback = dif - (c2.insertions.size() - c1.insertions.size());
		if (pullback > 0)
			pullBackInsertions(c1, pullback);
	}

	// keep track of which indexes from insertions vector were added at this step so we don't treat them again:
	bool c1_added[2*WorldConst::MaxGenomeLengthDifference] {false};
	bool c2_added[2*WorldConst::MaxGenomeLengthDifference] {false};
	// now do the insertions:
	decltype(c1.insertions) &ins1 = c1.insertions;
	decltype(c2.insertions) &ins2 = c2.insertions;

	for (uint i=0, j=0; i<ins1.size() || j<ins2.size(); ) {
		while (i<ins1.size() && c1_added[i])
			i++;
		while (j<ins2.size() && c2_added[j])
			j++;
		bool fromFirst = i<ins1.size();
		if (fromFirst && j<ins2.size()) {
			if (ins1[i].index == ins2[j].index) {
				// same position in both, just skip it
				i++, j++;
				continue;
			}
			fromFirst = ins1[i].index < ins2[j].index;		// choose the smallest insertion position first
		}
		if (fromFirst) {
			// insert the current insertion from first to second;
			int c2ListIndex = insertNewGene(c2, ins1[i], GeneNoOp());
			c2_added[c2ListIndex] = true;
		} else if (j<ins2.size()) {
			// insert the current insertion from second to first
			int c1ListIndex = insertNewGene(c1, ins2[j], GeneNoOp());
			c1_added[c1ListIndex] = true;
		}
		i++, j++;
	}
	trimInsertionList(c1);
	trimInsertionList(c2);
}

/*
 * 	1. mutating existing genes by altering their data and swapping positions
 * 	2. creating new genes
 * 	3. deleting existing genes
 * 	4. altering the meta-genes for all genes except new ones
 */
void GeneticOperations::alterChromosome(Chromosome &c) {
	LOGPREFIX("GeneticOperations");
#define ENABLE_STATS 1
#if(ENABLE_STATS)
	int stat_mutations = 0;
	int stat_swaps = 0;
	int stat_delete = 0;
	int stat_new = 0;
#endif

	static constexpr float numberMutationsPerChromosome = 0.125f;	// how many mutations we desire for a chromosome at most, on average
	static constexpr float numberSwapsPerChromosome = 0.0625f;
	static constexpr float numberDeletionsPerChromosome = 0.025f;

	// compute the total chance for mutations in the current chromosome:
	// also count the number of neurons, we need these in order to create new random genes
	float totalChanceToMutate = 0.f;
	float totalChanceToSwap = 0.f;
	float totalChanceToDelete = 0.f;
	for (unsigned i=0; i<c.genes.size(); i++) {
		float mutateCh, swapCh, deleteCh;
		getAlterationChances(c.genes[i], mutateCh, swapCh, deleteCh);
		totalChanceToMutate += mutateCh;
		totalChanceToSwap += swapCh;
		totalChanceToDelete += deleteCh;
	}

	// now we compute a factor to multiply the mutation chances to bring them into the desired range
	float mutationChanceFactor = std::min(1.f, numberMutationsPerChromosome / totalChanceToMutate);
	float swapChanceFactor = std::min(1.f, numberSwapsPerChromosome / totalChanceToSwap);
	float deleteChanceFactor = std::min(1.f, numberDeletionsPerChromosome / totalChanceToDelete);

	// now we go ahead with alterations:
	for (unsigned i=0; i<c.genes.size(); i++) {
		bool delGene = randf() < c.genes[i].chance_to_delete.value * deleteChanceFactor;
		if (delGene) {
			c.genes[i].type = gene_type::NO_OP;
#if(ENABLE_STATS)
			stat_delete++;
#endif
			continue;
		}
		bool swap = randf() < c.genes[i].chance_to_swap.value * swapChanceFactor;
		bool swapReverse = false;
		if (swap) {
			if (i < c.genes.size()-1) { // swap ahead
				xchg(c.genes[i], c.genes[i+1]);
#if(ENABLE_STATS)
				stat_swaps++;
#endif
			} else if (i > 0) { // swap behind
				xchg(c.genes[i], c.genes[i-1]);
				swapReverse = true;
#if(ENABLE_STATS)
				stat_swaps++;
#endif
			}
		}
		if (swapReverse) {
#if(ENABLE_STATS)
			stat_mutations +=
#endif
			alterGene(c.genes[i-1], mutationChanceFactor);
		} else {
#if(ENABLE_STATS)
			stat_mutations +=
#endif
			alterGene(c.genes[i], mutationChanceFactor);
		}

		if (swap && !swapReverse) {
			// swapped gene has been altered partially (by swapping), so must not go through a complete step again, do the rest here:
#if(ENABLE_STATS)
			stat_mutations +=
#endif
			alterGene(c.genes[i+1], mutationChanceFactor);
			i++; // skip the next gene that has already been altered
		}
	}

	// now there's a chance to spawn a new gene
	if (//false && // DEBUG: disable adding new genes
			randf() < constants::global_chance_to_spawn_gene * c.genes.size()) {
		int position = randi(c.genes.size()-1);
		Gene newGene(Gene::createRandom(c.genes.size()-position));
		if (c.genes[position].type == gene_type::NO_OP)
			c.genes[position] = newGene;
		else {
			// must keep a record of last genes inserted (at most N, and if gametes have a difference of more than N genes, they don't fuse)
			// when combining two gametes we must insert dummy genes at correspondent positions in the other chromosome, in order to realign the alelles.
			insertNewGene(c, Chromosome::insertion(position, 0), newGene);
		}
#if(ENABLE_STATS)
		stat_new++;
#endif
	}

#if(ENABLE_STATS)
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
	throw std::runtime_error("Implement!");
	int altered = 0;
	switch (g.type) {
	case gene_type::STOP:
	case gene_type::NO_OP:
		break;
//	case gene_type::SKIP:
//		altered += alterAtom(g.data.gene_skip.count, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_skip.maxDepth, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_skip.minDepth, mutationChanceFactor);
//		break;
//	case gene_type::BODY_ATTRIBUTE:
//		altered += alterAtom(g.data.gene_body_attribute.value, mutationChanceFactor);
//		break;
//	case gene_type::PROTEIN:
//		altered += alterAtom(g.data.gene_protein.maxDepth, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_protein.minDepth, mutationChanceFactor);
////		altered += alterAtom(g.data.gene_protein.protein, mutationChanceFactor);
////		altered += alterAtom(g.data.gene_protein.targetSegment, mutationChanceFactor);
//		break;
//	case gene_type::OFFSET:
//		altered += alterAtom(g.data.gene_offset.maxDepth, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_offset.minDepth, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_offset.offset, mutationChanceFactor);
////		altered += alterAtom(g.data.gene_offset.targetSegment, mutationChanceFactor);
//		break;
//	case gene_type::NEURON_INPUT_COORD:
//		altered += alterAtom(g.data.gene_neuron_input.destNeuronVirtIndex, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_neuron_input.inCoord, mutationChanceFactor);
//		break;
//	case gene_type::NEURON_OUTPUT_COORD:
//		altered += alterAtom(g.data.gene_neuron_output.srcNeuronVirtIndex, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_neuron_output.outCoord, mutationChanceFactor);
//		break;
//	case gene_type::NEURAL_BIAS:
//		altered += alterAtom(g.data.gene_neural_constant.targetNeuron, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_neural_constant.value, mutationChanceFactor);
//		break;
//	case gene_type::PART_ATTRIBUTE:
//		altered += alterAtom(g.data.gene_attribute.value, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_attribute.minDepth, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_attribute.maxDepth, mutationChanceFactor);
//		break;
//	case gene_type::SYNAPSE:
//		altered += alterAtom(g.data.gene_synapse.from, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_synapse.to, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_synapse.weight, mutationChanceFactor);
//		break;
//	case gene_type::TRANSFER_FUNC:
//		altered += alterAtom(g.data.gene_transfer_function.functionID, mutationChanceFactor);
//		altered += alterAtom(g.data.gene_transfer_function.targetNeuron, mutationChanceFactor);
//		break;
	/*case gene_type::JOINT_OFFSET:
		altered += alterAtom(g.data.gene_joint_offset.offset, mutationChanceFactor);
		altered += alterAtom(g.data.gene_joint_offset.minDepth, mutationChanceFactor);
		altered += alterAtom(g.data.gene_joint_offset.maxDepth, mutationChanceFactor);
		break;*/
	default:
		ERROR("unhandled gene type (alterGene): "<<(uint)g.type);
		break;
	}

	for (auto m : g.metaGenes) {
		alterMetaGene(*m);
	}

	return altered;
}

void GeneticOperations::getAlterationChances(Gene const& g, float& mutationCh, float& swapCh, float& deleteCh) {
	throw std::runtime_error("Implement!");
	deleteCh = g.chance_to_delete.value;
	swapCh = g.chance_to_swap.value;
	mutationCh = 0;
	switch (g.type) {
	case gene_type::STOP:
	case gene_type::NO_OP:
		break;
//	case gene_type::SKIP:
//		mutationCh += g.data.gene_skip.count.chanceToMutate.value;
//		mutationCh += g.data.gene_skip.maxDepth.chanceToMutate.value;
//		mutationCh += g.data.gene_skip.minDepth.chanceToMutate.value;
//		break;
//	case gene_type::BODY_ATTRIBUTE:
//		mutationCh += g.data.gene_body_attribute.value.chanceToMutate.value;
//		break;
//	case gene_type::PROTEIN:
//		mutationCh += g.data.gene_protein.maxDepth.chanceToMutate.value;
//		mutationCh += g.data.gene_protein.minDepth.chanceToMutate.value;
////		mutationCh += g.data.gene_protein.protein.chanceToMutate.value;
////		mutationCh += g.data.gene_protein.targetSegment.chanceToMutate.value;
//		break;
//	case gene_type::OFFSET:
//		mutationCh += g.data.gene_offset.maxDepth.chanceToMutate.value;
//		mutationCh += g.data.gene_offset.minDepth.chanceToMutate.value;
//		mutationCh += g.data.gene_offset.offset.chanceToMutate.value;
////		mutationCh += g.data.gene_offset.targetSegment.chanceToMutate.value;
//		break;
//	case gene_type::NEURON_INPUT_COORD:
//		mutationCh += g.data.gene_neuron_input.destNeuronVirtIndex.chanceToMutate.value;
//		mutationCh += g.data.gene_neuron_input.inCoord.chanceToMutate.value;
//		break;
//	case gene_type::NEURON_OUTPUT_COORD:
//		mutationCh += g.data.gene_neuron_output.srcNeuronVirtIndex.chanceToMutate.value;
//		mutationCh += g.data.gene_neuron_output.outCoord.chanceToMutate.value;
//		break;
//	case gene_type::NEURAL_BIAS:
//		mutationCh += g.data.gene_neural_constant.value.chanceToMutate.value;
//		mutationCh += g.data.gene_neural_constant.targetNeuron.chanceToMutate.value;
//		break;
//	case gene_type::PART_ATTRIBUTE:
//		mutationCh += g.data.gene_attribute.value.chanceToMutate.value;
//		break;
//	case gene_type::SYNAPSE:
//		mutationCh += g.data.gene_synapse.from.chanceToMutate.value;
//		mutationCh += g.data.gene_synapse.to.chanceToMutate.value;
//		mutationCh += g.data.gene_synapse.weight.chanceToMutate.value;
//		break;
//	case gene_type::TRANSFER_FUNC:
//		mutationCh += g.data.gene_transfer_function.functionID.chanceToMutate.value;
//		mutationCh += g.data.gene_transfer_function.targetNeuron.chanceToMutate.value;
//		break;
	/*case gene_type::JOINT_OFFSET:
		mutationCh += g.data.gene_joint_offset.offset.chanceToMutate.value;
		mutationCh += g.data.gene_joint_offset.minDepth.chanceToMutate.value;
		mutationCh += g.data.gene_joint_offset.maxDepth.chanceToMutate.value;
		break;*/
	default:
		ERROR("unhandled gene type (getAlterationChances): "<<(uint)g.type);
		break;
	}
}


void GeneticOperations::alterMetaGene(MetaGene &meta)
{
	meta.value += srandf() * meta.dynamic_variation;
	if (meta.value < 0)
		meta.value = 0;
}

