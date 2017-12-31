#include "Gene.h"
#include "../body-parts/BodyConst.h"
//#include "../body-parts/BodyPart.h"
#include "../neuralnet/functions.h"

#include <boglfw/utils/log.h>
#include <boglfw/math/math3D.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

void Gene::update_meta_genes_vec() {
	metaGenes.clear();
	// add meta-genes to this vector to enable their mutation:
	metaGenes.push_back(&chance_to_delete);
	metaGenes.push_back(&chance_to_swap);

	throw std::runtime_error("Implement!");

	switch (type) {
	/*case gene_type::PROTEIN:
		metaGenes.push_back(&data.gene_protein.maxDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_protein.maxDepth.changeAmount);
		metaGenes.push_back(&data.gene_protein.minDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_protein.minDepth.changeAmount);
		metaGenes.push_back(&data.gene_protein.protein.chanceToMutate);
		metaGenes.push_back(&data.gene_protein.protein.changeAmount);
		metaGenes.push_back(&data.gene_protein.weight.chanceToMutate);
		metaGenes.push_back(&data.gene_protein.weight.changeAmount);
		break;
	case gene_type::OFFSET:
		metaGenes.push_back(&data.gene_offset.maxDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_offset.maxDepth.changeAmount);
		metaGenes.push_back(&data.gene_offset.minDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_offset.minDepth.changeAmount);
		metaGenes.push_back(&data.gene_offset.offset.chanceToMutate);
		metaGenes.push_back(&data.gene_offset.offset.changeAmount);
		metaGenes.push_back(&data.gene_offset.side.chanceToMutate);
		metaGenes.push_back(&data.gene_offset.side.changeAmount);
		break;
	/*case gene_type::JOINT_OFFSET:
		metaGenes.push_back(&data.gene_joint_offset.maxDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_joint_offset.maxDepth.changeAmount);
		metaGenes.push_back(&data.gene_joint_offset.minDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_joint_offset.minDepth.changeAmount);
		metaGenes.push_back(&data.gene_joint_offset.offset.chanceToMutate);
		metaGenes.push_back(&data.gene_joint_offset.offset.changeAmount);
		break; * /
	case gene_type::PART_ATTRIBUTE:
		metaGenes.push_back(&data.gene_attribute.maxDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_attribute.maxDepth.changeAmount);
		metaGenes.push_back(&data.gene_attribute.minDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_attribute.minDepth.changeAmount);
		metaGenes.push_back(&data.gene_attribute.value.chanceToMutate);
		metaGenes.push_back(&data.gene_attribute.value.changeAmount);
		break;
	case gene_type::SYNAPSE:
		metaGenes.push_back(&data.gene_synapse.from.chanceToMutate);
		metaGenes.push_back(&data.gene_synapse.from.changeAmount);
		metaGenes.push_back(&data.gene_synapse.to.chanceToMutate);
		metaGenes.push_back(&data.gene_synapse.to.changeAmount);
		metaGenes.push_back(&data.gene_synapse.weight.chanceToMutate);
		metaGenes.push_back(&data.gene_synapse.weight.changeAmount);
		metaGenes.push_back(&data.gene_synapse.priority.chanceToMutate);
		metaGenes.push_back(&data.gene_synapse.priority.changeAmount);
		break;
	case gene_type::NEURON_INPUT_COORD:
		metaGenes.push_back(&data.gene_neuron_input.destNeuronVirtIndex.chanceToMutate);
		metaGenes.push_back(&data.gene_neuron_input.destNeuronVirtIndex.changeAmount);
		metaGenes.push_back(&data.gene_neuron_input.inCoord.chanceToMutate);
		metaGenes.push_back(&data.gene_neuron_input.inCoord.changeAmount);
		break;
	case gene_type::NEURON_OUTPUT_COORD:
		metaGenes.push_back(&data.gene_neuron_output.srcNeuronVirtIndex.chanceToMutate);
		metaGenes.push_back(&data.gene_neuron_output.srcNeuronVirtIndex.changeAmount);
		metaGenes.push_back(&data.gene_neuron_output.outCoord.chanceToMutate);
		metaGenes.push_back(&data.gene_neuron_output.outCoord.changeAmount);
		break;
	case gene_type::TRANSFER_FUNC:
		metaGenes.push_back(&data.gene_transfer_function.targetNeuron.chanceToMutate);
		metaGenes.push_back(&data.gene_transfer_function.targetNeuron.changeAmount);
		metaGenes.push_back(&data.gene_transfer_function.functionID.chanceToMutate);
		metaGenes.push_back(&data.gene_transfer_function.functionID.changeAmount);
		break;
	case gene_type::NEURAL_BIAS:
		metaGenes.push_back(&data.gene_neural_constant.targetNeuron.chanceToMutate);
		metaGenes.push_back(&data.gene_neural_constant.targetNeuron.changeAmount);
		metaGenes.push_back(&data.gene_neural_constant.value.chanceToMutate);
		metaGenes.push_back(&data.gene_neural_constant.value.changeAmount);
		break;
	case gene_type::NEURAL_PARAM:
		metaGenes.push_back(&data.gene_neural_param.targetNeuron.chanceToMutate);
		metaGenes.push_back(&data.gene_neural_param.targetNeuron.changeAmount);
		metaGenes.push_back(&data.gene_neural_param.value.chanceToMutate);
		metaGenes.push_back(&data.gene_neural_param.value.changeAmount);
		break;
	case gene_type::BODY_ATTRIBUTE:
		metaGenes.push_back(&data.gene_body_attribute.value.chanceToMutate);
		metaGenes.push_back(&data.gene_body_attribute.value.changeAmount);
		break;*/
	default:
		break;
	}
}

Gene Gene::createRandomBodyAttribGene() {
	GeneBodyAttribute g;
	g.attribute = (gene_body_attribute_type)randi(GENE_BODY_ATTRIB_INVALID+1, GENE_BODY_ATTRIB_END-1);
	switch (g.attribute) {
	case GENE_BODY_ATTRIB_ADULT_LEAN_MASS:
		g.value.set(BodyConst::initialAdultLeanMass);
		break;
	case GENE_BODY_ATTRIB_EGG_MASS:
		g.value.set(BodyConst::initialEggMass);
		break;
	case GENE_BODY_ATTRIB_GROWTH_SPEED:
		g.value.set(BodyConst::initialGrowthSpeed);
		break;
//	case GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO:
//		g.value.set(BodyConst::initialFatMassRatio);
//		break;
	case GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO:
		g.value.set(BodyConst::initialMinFatMassRatio);
		break;
	case GENE_BODY_ATTRIB_REPRODUCTIVE_MASS_RATIO:
		g.value.set(BodyConst::initialReproductiveMassRatio);
		break;
	default:
		ERROR("unhandled body attrib type: " << g.attribute);
	}
	return g;
}

Gene Gene::createRandomProteinGene() {
	throw std::runtime_error("Implement!");
	GeneProtein g;
//	g.maxDepth.set(randi(8));
//	g.minDepth.set(0);
//	g.protein.set((gene_protein_type)randi(GENE_PROT_NONE+1, GENE_PROT_END-1));
//	g.weight.set(randf());
	return g;
}

Gene Gene::createRandomOffsetGene(int spaceLeftAfter) {
	throw std::runtime_error("Implement!");
	GeneOffset g;
//	g.maxDepth.set(randi(5));
//	g.minDepth.set(0);
//	g.side.set(srandf());
//	g.offset.set(randi(spaceLeftAfter));
	return g;
}

/*Gene Gene::createRandomJointOffsetGene(int spaceLeftAfter) {
	GeneJointOffset g;
	g.maxDepth.set(randi(5));
	g.minDepth.set(0);
	g.offset.set(randi(spaceLeftAfter));
	return g;
}*/

Gene Gene::createRandomSynapseGene(int nNeurons) {
	throw std::runtime_error("Implement!");
	GeneSynapse g;
//	g.from.set(randi(nNeurons-1));
//	g.to.set(randi(nNeurons-1));
//	g.weight.set(randf()*0.2f);
//	g.priority.set(randf()*10);
	return g;
}

Gene Gene::createRandomNeuronInputCoordGene(int nNeurons) {
	throw std::runtime_error("Implement!");
	GeneNeuronInputCoord g;
//	g.destNeuronVirtIndex.set(randi(nNeurons-1));
//	g.inCoord.set(randf() * BodyConst::MaxVMSCoordinateValue);
	return g;
}

Gene Gene::createRandomNeuronOutputCoordGene(int nNeurons) {
	throw std::runtime_error("Implement!");
	GeneNeuronOutputCoord g;
//	g.srcNeuronVirtIndex.set(randi(nNeurons-1));
//	g.outCoord.set(randf() * BodyConst::MaxVMSCoordinateValue);
	return g;
}

Gene Gene::createRandomNeuralBiasGene(int nNeurons) {
	throw std::runtime_error("Implement!");
	GeneNeuralBias g;
//	g.targetNeuron.set(randi(nNeurons-1));
//	g.value.set(srandf());
	return g;
}

Gene Gene::createRandomNeuralParamGene(int nNeurons) {
	throw std::runtime_error("Implement!");
	GeneNeuralParam g;
//	g.targetNeuron.set(randi(nNeurons-1));
//	g.value.set(srandf());
	return g;
}

Gene Gene::createRandomTransferFuncGene(int nNeurons) {
	throw std::runtime_error("Implement!");
	GeneTransferFunction g;
//	g.functionID.set(randi((int)transferFuncNames::FN_MAXCOUNT-1));
//	g.targetNeuron.set(randi(nNeurons-1));
	return g;
}

Gene Gene::createRandomAttribGene() {
	GeneAttribute g;
	g.attribute = (gene_part_attribute_type)randi(GENE_ATTRIB_INVALID+1, GENE_ATTRIB_END-1);
	g.value.set(randf());
	return g;
}

Gene Gene::createRandomSkipGene(int spaceLeftAfter) {
	throw std::runtime_error("Implement!");
	GeneSkip g;
//	g.minDepth.set(randi(10));
//	g.maxDepth.set(g.minDepth + randi(10-g.minDepth));
	// use a random distribution that favors small values:
//	g.count.set(sqr(randd()) * spaceLeftAfter);
	return g;
}

Gene Gene::createRandom(int spaceLeftAfter, int nNeurons) {
	std::vector<std::pair<gene_type, double>> geneChances {
		// these are relative chances:
		{gene_type::BODY_ATTRIBUTE, 1.0},
		{gene_type::PROTEIN, 1.5},
		{gene_type::PART_ATTRIBUTE, 2.1},
		{gene_type::OFFSET, 0.3},
		//{gene_type::JOINT_OFFSET, 0.3},
		{gene_type::NEURAL_BIAS, 1.0},
		{gene_type::NEURAL_PARAM, 0.8},
		{gene_type::TRANSFER_FUNC, 0.5},
		{gene_type::SYNAPSE, 1.5},
		{gene_type::NEURON_INPUT_COORD, 0.5},
		{gene_type::NEURON_OUTPUT_COORD, 0.5},
		{gene_type::SKIP, 0.12},
#ifdef ENABLE_START_MARKER_GENES
		{gene_type::START_MARKER, 0.1},
#endif
		{gene_type::STOP, 0.09},
		{gene_type::NO_OP, 0.09},
	};
	// normalize chances to make them sum up to 1.0
	double total = 0;
	for (auto &x : geneChances)
		total += x.second;
	for (auto &x : geneChances)
		x.second /= total;
	double dice = randd();
	double floor = 0;
	gene_type type = gene_type::INVALID;
	for (auto &x : geneChances) {
		if (dice - floor < x.second) {
			type = x.first;
			break;
		}
		floor += x.second;
	}
	switch (type) {
	case gene_type::BODY_ATTRIBUTE:
		return createRandomBodyAttribGene();
	case gene_type::PROTEIN:
		return createRandomProteinGene();
	case gene_type::OFFSET:
		return createRandomOffsetGene(spaceLeftAfter);
	/*case gene_type::JOINT_OFFSET:
		return createRandomJointOffsetGene(spaceLeftAfter);*/
	case gene_type::NEURON_INPUT_COORD:
		return createRandomNeuronInputCoordGene(nNeurons);
	case gene_type::NEURON_OUTPUT_COORD:
		return createRandomNeuronOutputCoordGene(nNeurons);
	case gene_type::NEURAL_BIAS:
		return createRandomNeuralBiasGene(nNeurons);
	case gene_type::PART_ATTRIBUTE:
		return createRandomAttribGene();
	case gene_type::SKIP:
		return createRandomSkipGene(spaceLeftAfter);
#ifdef ENABLE_START_MARKER_GENES
	case gene_type::START_MARKER:
		return GeneStartMarker();
#endif
	case gene_type::STOP:
		return GeneStop();
	case gene_type::SYNAPSE:
		return createRandomSynapseGene(nNeurons);
	case gene_type::TRANSFER_FUNC:
		return createRandomTransferFuncGene(nNeurons);
	case gene_type::NO_OP:
		return GeneNoOp();
	default:
		ERROR("unhandled gene random type: " << (uint)type);
		return GeneStop();
	}
}

char Gene::getSymbol() const {
	switch (type) {
	case gene_type::BODY_ATTRIBUTE:
		return 'B';
	/*case gene_type::JOINT_OFFSET:
		return 'J';*/
	case gene_type::NEURAL_BIAS:
		return 'C';
	case gene_type::NEURON_INPUT_COORD:
		return 'I';
	case gene_type::NEURON_OUTPUT_COORD:
		return 'O';
	case gene_type::NEURAL_PARAM:
		return 'N';
	case gene_type::NO_OP:
		return '_';
	case gene_type::OFFSET:
		return '@';
	case gene_type::PART_ATTRIBUTE:
		return 'A';
	case gene_type::PROTEIN:
		return 'P';
	case gene_type::SKIP:
		return '>';
#ifdef ENABLE_START_MARKER_GENES
	case gene_type::START_MARKER:
		return ':';
#endif
	case gene_type::STOP:
		return '!';
	case gene_type::SYNAPSE:
		return 'S';
	case gene_type::TRANSFER_FUNC:
		return 'T';
	default:
		return '?';
	}
}
