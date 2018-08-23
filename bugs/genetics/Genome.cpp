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
	unsigned p = 0;
	for (unsigned i=0; i<genes.size(); i++) {
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
		if (i<gen.first.genes.size()
				&& gen.first.genes[i].type != gene_type::NO_OP)			// avoid no-op genes
		{
			g = &gen.first.genes[i];
			if (i<gen.second.genes.size() && randf() < 0.5f
					&& gen.second.genes[i].type != gene_type::NO_OP)	// avoid no-op genes
				g = &gen.second.genes[i];
		} else if (i<gen.second.genes.size())
			g = &gen.second.genes[i];

		if (g)
			c.genes.push_back(*g);
		i++;
	}
	// copy insertion list:
	c.insertions = gen.first.insertions;	// both chromosomes should have identical lists
	// increase all insertions' age:
	for (unsigned i=0; i<c.insertions.size(); i++)
		c.insertions[i].age++;
	// perform some mutations:
	alterChromosome(c);
	trimInsertionList(c);
	return c;
}

/*
 * this will insert a new gene and return the index in the insertions vector where this change has been recorded
 */
int GeneticOperations::insertNewGene(Chromosome &c, Chromosome::insertion ins, Gene const& g) {
	assertDbg(ins.index <= (int)c.genes.size());
	c.genes.insert(c.genes.begin() + ins.index, g);
	// determine where in insertions we must add this new index
	unsigned d=0;
	while (d<c.insertions.size() && c.insertions[d].index < ins.index) d++;
	c.insertions.insert(c.insertions.begin()+d, ins);
	int ret = d;
	// increment all insertions that are to the right of this one
	for (++d; d<c.insertions.size(); d++)
		c.insertions[d].index++;
	return ret;
}

// with extra=0 insertions will be trimmed to constants::MaxGenomeLengthDifference
// with extra>0 additional oldest insertions will be removed
void GeneticOperations::trimInsertionList(Chromosome &c, unsigned extra) {
	assertDbg(extra <= c.insertions.size());
	while (c.insertions.size()+extra > constants::MaxGenomeLengthDifference) {
		// search for oldest entry and remove it
		unsigned ioldest = 0;
		for (unsigned i=1; i<c.insertions.size(); i++)
			if (c.insertions[i].age > c.insertions[ioldest].age)
				ioldest = i;
		c.insertions.erase(c.insertions.begin()+ioldest);
	}
}

void GeneticOperations::fixGenesSynchro(Genome& gen) {
	// this shit is more complicated than i thought
	// ^^ several years later i had to change it so that it won't assume both chromosomes started out as the same length
	// so... yeah
	DEBUGLOGLN("chromosome diff: "<< (int)abs(gen.first.genes.size() - (int)gen.second.genes.size()));
	assertDbg((unsigned)abs((int)gen.first.genes.size() - (int)gen.second.genes.size()) <= constants::MaxGenomeLengthDifference);

	// assumption: insertions list from each chromosome should be sorted from left to right (smallest index first)
	Chromosome &c1 = gen.first;
	Chromosome &c2 = gen.second;

	// compute the length difference between chromosomes:
	int dif = (int)c1.genes.size() - (int)c2.genes.size();
	int ins_dif = (int)c1.insertions.size() - (int)c2.insertions.size();
	Chromosome *cshort = nullptr;
	Chromosome *clong = nullptr;
	// remove oldest insertions on the shorter chromosome until the difference in size matches the difference
	// in number of insertions:
	if (dif > 0) {
		//assertDbg(c1.insertions.size() >= c2.insertions.size());	// not necessary because the two chromosomes may have had different lengths from the beginning
		// C2 is shorter
		int amount = dif - ins_dif;
		if (amount > 0)
			trimInsertionList(c2, min(amount, (int)c2.insertions.size()));
		cshort = &c2;
		clong = &c1;
	} else if (dif < 0) {
		// C1 is shorter
		//assertDbg(c1.insertions.size() <= c2.insertions.size());
		int amount = dif + ins_dif;
		if (amount > 0)
			trimInsertionList(c1, min(amount, (int)c1.insertions.size()));
		cshort = &c1;
		clong = &c2;
	} else {
		cshort = ins_dif >= 0 ? &c1 : &c2;
		clong = ins_dif >= 0 ? &c2 : &c1;
	}
	// recompute insertions difference and add padding to the shorter chromosome until the gene difference is equal to insertions difference
	ins_dif = (int)c1.insertions.size() - (int)c2.insertions.size();
	if (sign(ins_dif) == sign(dif)) {	// only if the shorter chromosome has fewer insertions
		for (; abs(dif) > abs(ins_dif); dif -= sign(dif)) {
			// 50-50% chance of either padding the shorter chromosome or discarding the latest gene from the longer one
			if (randf() < 0.5)
				// do padding
				cshort->genes.push_back(GeneNoOp{});
			else {
				// discard the youngest gene from the long chromosome:
				int inew = 0;
				for (unsigned i=1; i<clong->insertions.size(); i++)
					if (clong->insertions[i].age < clong->insertions[inew].age)
						inew = i;
				clong->genes.erase(clong->genes.begin() + clong->insertions[inew].index);
				clong->insertions.erase(clong->insertions.begin() + inew);
			}
		}
	}

	// keep track of which indexes from insertions vector were added at this step so we don't treat them again:
	bool c1_added[2*constants::MaxGenomeLengthDifference] {false};
	bool c2_added[2*constants::MaxGenomeLengthDifference] {false};
	// now do the insertions:
	decltype(c1.insertions) &ins1 = c1.insertions;
	decltype(c2.insertions) &ins2 = c2.insertions;

	for (unsigned i=0, j=0; i<ins1.size() || j<ins2.size(); ) {
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
			if ((unsigned)ins1[i].index <= c2.genes.size()) {
				int c2ListIndex = insertNewGene(c2, ins1[i], GeneNoOp());
				c2_added[c2ListIndex] = true;
			}
		} else if (j<ins2.size()) {
			// insert the current insertion from second to first
			if ((unsigned)ins2[j].index <= c1.genes.size()) {
				int c1ListIndex = insertNewGene(c1, ins2[j], GeneNoOp());
				c1_added[c1ListIndex] = true;
			}
		}
		i++, j++;
	}
	trimInsertionList(c1);
	trimInsertionList(c2);

	// we're done with insertions, now let's remove homologous no-op genes to keep the genome lean and mean
	// we need to track and update offset genes that may be affected by removing the no-op genes
	std::vector<int> c1_offset_genes;	// holds the indexes of offset genes
	std::vector<int> c2_offset_genes;
	for (unsigned i=0; i<c1.genes.size() && i<c2.genes.size(); i++) {
		if (c1.genes[i].type == gene_type::OFFSET)
			c1_offset_genes.push_back(i);
		if (c2.genes[i].type == gene_type::OFFSET)
			c2_offset_genes.push_back(i);
		if (c1.genes[i].type == gene_type::NO_OP && c2.genes[i].type == gene_type::NO_OP) {
			// we remove this gene from both chromosomes and then fix insertion indexes
			c1.genes.erase(c1.genes.begin()+i);
			c2.genes.erase(c2.genes.begin()+i);
			// update all insertions indexes that follow (if this was an insertion we remove it, otherwise we update the insertion index)
			for (unsigned j=0; j<c1.insertions.size(); j++)
				if (c1.insertions[j].index == i) {
					c1.insertions.erase(c1.insertions.begin()+j);
					j--;
				}
				else if (c1.insertions[j].index > i)
					c1.insertions[j].index--;
			for (unsigned j=0; j<c2.insertions.size(); j++)
				if (c2.insertions[j].index == i) {
					c2.insertions.erase(c2.insertions.begin()+j);
					j--;
				}
				else if (c2.insertions[j].index > i)
					c2.insertions[j].index--;
			// update offset genes that might have been affected:
			for (unsigned j=0; j<c1_offset_genes.size(); j++) {
				int offset_value = c1.genes[c1_offset_genes[j]].data.gene_offset.offset;
				int abs_offset_target = c1_offset_genes[j] + offset_value;
				if (abs_offset_target > i)
					c1.genes[c1_offset_genes[j]].data.gene_offset.offset.value--;
				else {
					// the offset gene's target was before the current genome position, so we can forget about it:
					c1_offset_genes.erase(c1_offset_genes.begin() + j);
					j--;
				}
			}
			for (unsigned j=0; j<c2_offset_genes.size(); j++) {
				int offset_value = c2.genes[c2_offset_genes[j]].data.gene_offset.offset;
				int abs_offset_target = c2_offset_genes[j] + offset_value;
				if (abs_offset_target > i)
					c2.genes[c2_offset_genes[j]].data.gene_offset.offset.value--;
				else {
					// the offset gene's target was before the current genome position, so we can forget about it:
					c2_offset_genes.erase(c2_offset_genes.begin() + j);
					j--;
				}
			}
		}
	}
}

/*
 * 	1. mutating existing genes by altering their data and swapping positions
 * 	2. creating new genes
 * 	3. deleting existing genes
 * 	4. altering the meta-genes for all genes except new ones
 */
void GeneticOperations::alterChromosome(Chromosome &c) {
	LOGPREFIX("GeneticOperations");
#define ENABLE_STATS 0
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
			} else // this seems to be the only gene on the chromosome, wtf?!?!
				swap = false;
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
			randf() < constants::global_chance_to_spawn_gene) {
		int position = randi(c.genes.size()-1);
		Gene newGene(Gene::createRandom(c.genes.size()-position));
		if (c.genes[position].type == gene_type::NO_OP)
			c.genes[position] = newGene;
		else {
			// we keep a record of last genes inserted (at most N, and if gametes have a difference of more than N genes, they don't fuse)
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
int alterAtom(Atom<T> &a, float mutationChanceFactor) {
	float chance = a.chanceToMutate.value * mutationChanceFactor;
	if (chance < constants::global_alteration_override_chance)
		chance = constants::global_alteration_override_chance;
	if (randf() < chance) {
		if (std::is_integral<T>::value) {
			a.value += (randf()<0.5f) ? +1 : -1;
		} else {
			a.value += srandf() * a.changeAmount.value;
		}
		return 1;
	} else
		return 0;
}

int alterRestriction(BranchRestriction& r, float mutationChanceFactor) {
	int altered = alterAtom(r.activeLevels, mutationChanceFactor);
	for (unsigned i=0; i<constants::MAX_DIVISION_DEPTH; i++) {
		altered += alterAtom(r.levels[i].skipLeft, mutationChanceFactor);
		altered += alterAtom(r.levels[i].skipRight, mutationChanceFactor);
		altered += alterAtom(r.levels[i].stopLeft, mutationChanceFactor);
		altered += alterAtom(r.levels[i].stopRight, mutationChanceFactor);
	}
	return altered;
}

int GeneticOperations::alterGene(Gene &g, float mutationChanceFactor) {
	int altered = 0;
	switch (g.type) {
	case gene_type::STOP:
	case gene_type::NO_OP:
		break;
	case gene_type::BODY_ATTRIBUTE:
		altered += alterAtom(g.data.gene_body_attribute.value, mutationChanceFactor);
		break;
	case gene_type::DIVISION_PARAM:
		altered += alterAtom(g.data.gene_division_param.value, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_division_param.restriction, mutationChanceFactor);
		break;
	case gene_type::JOINT_ATTRIBUTE:
		altered += alterAtom(g.data.gene_joint_attrib.value, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_joint_attrib.restriction, mutationChanceFactor);
		break;
	case gene_type::MUSCLE_ATTRIBUTE:
		altered += alterAtom(g.data.gene_muscle_attrib.value, mutationChanceFactor);
		altered += alterAtom(g.data.gene_muscle_attrib.side, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_muscle_attrib.restriction, mutationChanceFactor);
		break;
	case gene_type::NEURAL_BIAS:
		altered += alterAtom(g.data.gene_neural_constant.neuronLocation, mutationChanceFactor);
		altered += alterAtom(g.data.gene_neural_constant.value, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_neural_constant.restriction, mutationChanceFactor);
		break;
	case gene_type::NEURAL_PARAM:
		altered += alterAtom(g.data.gene_neural_param.neuronLocation, mutationChanceFactor);
		altered += alterAtom(g.data.gene_neural_param.value, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_neural_param.restriction, mutationChanceFactor);
		break;
	case gene_type::NEURON:
		altered += alterAtom(g.data.gene_neuron.neuronLocation, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_neuron.restriction, mutationChanceFactor);
		break;
	case gene_type::OFFSET:
		altered += alterAtom(g.data.gene_offset.offset, mutationChanceFactor);
		altered += alterAtom(g.data.gene_offset.side, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_offset.restriction, mutationChanceFactor);
		break;
	case gene_type::PART_ATTRIBUTE:
		altered += alterAtom(g.data.gene_attribute.value, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_attribute.restriction, mutationChanceFactor);
		break;
	case gene_type::PROTEIN:
		altered += alterAtom(g.data.gene_protein.weight, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_protein.restriction, mutationChanceFactor);
		break;
	case gene_type::SKIP:
		altered += alterAtom(g.data.gene_skip.count, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_skip.restriction, mutationChanceFactor);
		break;
	case gene_type::SYNAPSE:
		altered += alterAtom(g.data.gene_synapse.destLocation, mutationChanceFactor);
		altered += alterAtom(g.data.gene_synapse.priority, mutationChanceFactor);
		altered += alterAtom(g.data.gene_synapse.srcLocation, mutationChanceFactor);
		altered += alterAtom(g.data.gene_synapse.weight, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_synapse.restriction, mutationChanceFactor);
		break;
	case gene_type::TIME_SYNAPSE:
		altered += alterAtom(g.data.gene_time_synapse.weight, mutationChanceFactor);
		altered += alterAtom(g.data.gene_time_synapse.targetLocation, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_time_synapse.restriction, mutationChanceFactor);
		break;
	case gene_type::TRANSFER_FUNC:
		altered += alterAtom(g.data.gene_transfer_function.functionID, mutationChanceFactor);
		altered += alterAtom(g.data.gene_transfer_function.neuronLocation, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_transfer_function.restriction, mutationChanceFactor);
		break;
	case gene_type::VMS_OFFSET:
		altered += alterAtom(g.data.gene_vms_offset.value, mutationChanceFactor);
		altered += alterRestriction(g.data.gene_vms_offset.restriction, mutationChanceFactor);
		break;
	default:
		ERROR("unhandled gene type (alterGene): "<<(unsigned)g.type);
		break;
	}

	for (auto m : g.metaGenes) {
		alterMetaGene(*m);
	}

	return altered;
}

float restrictionMutationChance(BranchRestriction const& r) {
	float val = 0;
	val += r.activeLevels.chanceToMutate.value;
	for (unsigned i=0; i<constants::MAX_DIVISION_DEPTH; i++) {
		val += r.levels[i].skipLeft.chanceToMutate.value;
		val += r.levels[i].skipRight.chanceToMutate.value;
		val += r.levels[i].stopLeft.chanceToMutate.value;
		val += r.levels[i].stopRight.chanceToMutate.value;
	}
	return val;
}

void GeneticOperations::getAlterationChances(Gene const& g, float& mutationCh, float& swapCh, float& deleteCh) {
	deleteCh = g.chance_to_delete.value;
	swapCh = g.chance_to_swap.value;
	mutationCh = 0;
	switch (g.type) {
	case gene_type::STOP:
	case gene_type::NO_OP:
		break;
	case gene_type::BODY_ATTRIBUTE:
		mutationCh += g.data.gene_body_attribute.value.chanceToMutate.value;
		break;
	case gene_type::DIVISION_PARAM:
		mutationCh += g.data.gene_division_param.value.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_division_param.restriction);
		break;
	case gene_type::JOINT_ATTRIBUTE:
		mutationCh += g.data.gene_joint_attrib.value.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_joint_attrib.restriction);
		break;
	case gene_type::MUSCLE_ATTRIBUTE:
		mutationCh += g.data.gene_muscle_attrib.value.chanceToMutate.value;
		mutationCh += g.data.gene_muscle_attrib.side.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_muscle_attrib.restriction);
		break;
	case gene_type::NEURAL_BIAS:
		mutationCh += g.data.gene_neural_constant.neuronLocation.chanceToMutate.value;
		mutationCh += g.data.gene_neural_constant.value.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_neural_constant.restriction);
		break;
	case gene_type::NEURAL_PARAM:
		mutationCh += g.data.gene_neural_param.neuronLocation.chanceToMutate.value;
		mutationCh += g.data.gene_neural_param.value.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_neural_param.restriction);
		break;
	case gene_type::NEURON:
		mutationCh += g.data.gene_neuron.neuronLocation.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_neuron.restriction);
		break;
	case gene_type::OFFSET:
		mutationCh += g.data.gene_offset.offset.chanceToMutate.value;
		mutationCh += g.data.gene_offset.side.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_offset.restriction);
		break;
	case gene_type::PART_ATTRIBUTE:
		mutationCh += g.data.gene_attribute.value.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_attribute.restriction);
		break;
	case gene_type::PROTEIN:
		mutationCh += g.data.gene_protein.weight.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_protein.restriction);
		break;
	case gene_type::SKIP:
		mutationCh += g.data.gene_skip.count.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_skip.restriction);
		break;
	case gene_type::SYNAPSE:
		mutationCh += g.data.gene_synapse.destLocation.chanceToMutate.value;
		mutationCh += g.data.gene_synapse.priority.chanceToMutate.value;
		mutationCh += g.data.gene_synapse.srcLocation.chanceToMutate.value;
		mutationCh += g.data.gene_synapse.weight.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_synapse.restriction);
		break;
	case gene_type::TIME_SYNAPSE:
		mutationCh += g.data.gene_time_synapse.weight.chanceToMutate.value;
		mutationCh += g.data.gene_time_synapse.targetLocation.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_time_synapse.restriction);
		break;
	case gene_type::TRANSFER_FUNC:
		mutationCh += g.data.gene_transfer_function.functionID.chanceToMutate.value;
		mutationCh += g.data.gene_transfer_function.neuronLocation.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_transfer_function.restriction);
		break;
	case gene_type::VMS_OFFSET:
		mutationCh += g.data.gene_vms_offset.value.chanceToMutate.value;
		mutationCh += restrictionMutationChance(g.data.gene_vms_offset.restriction);
		break;
	default:
		ERROR("unhandled gene type (getAlterationChances): "<<(unsigned)g.type);
		break;
	}
}


void GeneticOperations::alterMetaGene(MetaGene &meta)
{
	meta.value += srandf() * meta.dynamic_variation;
	if (meta.value < 0)
		meta.value = 0;
}

bool Chromosome::operator == (Chromosome const& c) const {
	if (genes.size() != c.genes.size())
		return false;
	if (insertions.size() != c.insertions.size())
		return false;
	for (unsigned i=0; i<genes.size(); i++)
		if (genes[i] != c.genes[i])
			return false;
	for (unsigned i=0; i<insertions.size(); i++)
		if (insertions[i] != c.insertions[i])
			return false;
	return true;
}
