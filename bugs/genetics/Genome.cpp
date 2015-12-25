/*
 * Genome.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: bog
 */

#include "Genome.h"
#include "../utils/rand.h"
#include "../utils/log.h"
#include "../math/math2D.h"
#include <set>

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
		/*if (g->type == GENE_TYPE_DEVELOPMENT)
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
#ifdef DEBUG
	int stat_mutations = 0;
	int stat_swaps = 0;
	int stat_delete = 0;
	int stat_new = 0;
#endif

	std::map<int, bool> mapNeuronsExist;

	static constexpr float numberAlterationsPerChromosome = 3;	// how many alterations we desire for a chromosome at most

	// compute the total chance for mutations in the current chromosome:
	// also count the number of neurons, motors and sensors since we need these in order to create new random genes
	float totalChanceToChange = 0.f;
	for (unsigned i=0; i<c.genes.size(); i++) {
		totalChanceToChange += getTotalMutationChance(c.genes[i]);

		// count
		switch (c.genes[i].type) {
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
	if (//false && // DEBUG: disable adding new genes
			randf() < constants::global_chance_to_spawn_gene * c.genes.size()) {
		int position = randi(c.genes.size()-1);
		Gene newGene(Gene::createRandom(c.genes.size()-position, nNeurons));
		if (c.genes[position].type == GENE_TYPE_NO_OP)
			c.genes[position] = newGene;
		else {
			// must keep a record of last genes inserted (at most N, and if gametes have a difference of more than N genes, they don't fuse)
			// when combining two gametes we must insert dummy genes at correspondent positions in the other chromosome, in order to realign the alelles.
			insertNewGene(c, Chromosome::insertion(position, 0), newGene);
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
	case GENE_TYPE_START_MARKER:
	case GENE_TYPE_STOP:
	case GENE_TYPE_NO_OP:
		break;
	case GENE_TYPE_SKIP:
		altered += alterAtom(g.data.gene_skip.count, mutationChanceFactor);
		altered += alterAtom(g.data.gene_skip.maxDepth, mutationChanceFactor);
		altered += alterAtom(g.data.gene_skip.minDepth, mutationChanceFactor);
		break;
	case GENE_TYPE_BODY_ATTRIBUTE:
		altered += alterAtom(g.data.gene_body_attribute.value, mutationChanceFactor);
		break;
	case GENE_TYPE_PROTEIN:
		altered += alterAtom(g.data.gene_protein.maxDepth, mutationChanceFactor);
		altered += alterAtom(g.data.gene_protein.minDepth, mutationChanceFactor);
		altered += alterAtom(g.data.gene_protein.protein, mutationChanceFactor);
		altered += alterAtom(g.data.gene_protein.targetSegment, mutationChanceFactor);
		break;
	case GENE_TYPE_OFFSET:
		altered += alterAtom(g.data.gene_offset.maxDepth, mutationChanceFactor);
		altered += alterAtom(g.data.gene_offset.minDepth, mutationChanceFactor);
		altered += alterAtom(g.data.gene_offset.offset, mutationChanceFactor);
		altered += alterAtom(g.data.gene_offset.targetSegment, mutationChanceFactor);
		break;
	case GENE_TYPE_NEURON_INPUT_COORD:
		altered += alterAtom(g.data.gene_neuron_input.destNeuronVirtIndex, mutationChanceFactor);
		altered += alterAtom(g.data.gene_neuron_input.inCoord, mutationChanceFactor);
		break;
	case GENE_TYPE_NEURON_OUTPUT_COORD:
		altered += alterAtom(g.data.gene_neuron_output.srcNeuronVirtIndex, mutationChanceFactor);
		altered += alterAtom(g.data.gene_neuron_output.outCoord, mutationChanceFactor);
		break;
	case GENE_TYPE_NEURAL_CONST:
		altered += alterAtom(g.data.gene_neural_constant.targetNeuron, mutationChanceFactor);
		altered += alterAtom(g.data.gene_neural_constant.value, mutationChanceFactor);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		altered += alterAtom(g.data.gene_attribute.value, mutationChanceFactor);
		altered += alterAtom(g.data.gene_attribute.minDepth, mutationChanceFactor);
		altered += alterAtom(g.data.gene_attribute.maxDepth, mutationChanceFactor);
		break;
	case GENE_TYPE_SYNAPSE:
		altered += alterAtom(g.data.gene_synapse.from, mutationChanceFactor);
		altered += alterAtom(g.data.gene_synapse.to, mutationChanceFactor);
		altered += alterAtom(g.data.gene_synapse.weight, mutationChanceFactor);
		break;
	case GENE_TYPE_TRANSFER_FUNC:
		altered += alterAtom(g.data.gene_transfer_function.functionID, mutationChanceFactor);
		altered += alterAtom(g.data.gene_transfer_function.targetNeuron, mutationChanceFactor);
		break;
	case GENE_TYPE_JOINT_OFFSET:
		altered += alterAtom(g.data.gene_joint_offset.offset, mutationChanceFactor);
		altered += alterAtom(g.data.gene_joint_offset.minDepth, mutationChanceFactor);
		altered += alterAtom(g.data.gene_joint_offset.maxDepth, mutationChanceFactor);
		break;
	default:
		ERROR("unhandled gene type (alterGene): "<<(uint)g.type);
		break;
	}

	for (auto m : g.metaGenes) {
		alterMetaGene(*m);
	}

	return altered;
}

float GeneticOperations::getTotalMutationChance(Gene const& g) {
	float ret = g.chance_to_delete.value + g.chance_to_swap.value;
	switch (g.type) {
	case GENE_TYPE_START_MARKER:
	case GENE_TYPE_STOP:
	case GENE_TYPE_NO_OP:
		break;
	case GENE_TYPE_SKIP:
		ret += g.data.gene_skip.count.chanceToMutate.value;
		ret += g.data.gene_skip.maxDepth.chanceToMutate.value;
		ret += g.data.gene_skip.minDepth.chanceToMutate.value;
		break;
	case GENE_TYPE_BODY_ATTRIBUTE:
		ret += g.data.gene_body_attribute.value.chanceToMutate.value;
		break;
	case GENE_TYPE_PROTEIN:
		ret += g.data.gene_protein.maxDepth.chanceToMutate.value;
		ret += g.data.gene_protein.minDepth.chanceToMutate.value;
		ret += g.data.gene_protein.protein.chanceToMutate.value;
		ret += g.data.gene_protein.targetSegment.chanceToMutate.value;
		break;
	case GENE_TYPE_OFFSET:
		ret += g.data.gene_offset.maxDepth.chanceToMutate.value;
		ret += g.data.gene_offset.minDepth.chanceToMutate.value;
		ret += g.data.gene_offset.offset.chanceToMutate.value;
		ret += g.data.gene_offset.targetSegment.chanceToMutate.value;
		break;
	case GENE_TYPE_NEURON_INPUT_COORD:
		ret += g.data.gene_neuron_input.destNeuronVirtIndex.chanceToMutate.value;
		ret += g.data.gene_neuron_input.inCoord.chanceToMutate.value;
		break;
	case GENE_TYPE_NEURON_OUTPUT_COORD:
		ret += g.data.gene_neuron_output.srcNeuronVirtIndex.chanceToMutate.value;
		ret += g.data.gene_neuron_output.outCoord.chanceToMutate.value;
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
	case GENE_TYPE_TRANSFER_FUNC:
		ret += g.data.gene_transfer_function.functionID.chanceToMutate.value;
		ret += g.data.gene_transfer_function.targetNeuron.chanceToMutate.value;
		break;
	case GENE_TYPE_JOINT_OFFSET:
		ret += g.data.gene_joint_offset.offset.chanceToMutate.value;
		ret += g.data.gene_joint_offset.minDepth.chanceToMutate.value;
		ret += g.data.gene_joint_offset.maxDepth.chanceToMutate.value;
		break;
	default:
		ERROR("unhandled gene type (getTotalMutationChance): "<<(uint)g.type);
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

