#include "Gene.h"
#include "../utils/log.h"
#include "../body-parts/BodyConst.h"
#include "../body-parts/BodyPart.h"
#include "../neuralnet/functions.h"

void Gene::update_meta_genes_vec() {
	metaGenes.clear();
	// add meta-genes to this vector to enable their mutation:
	metaGenes.push_back(&chance_to_delete);
	metaGenes.push_back(&chance_to_swap);

	switch (type) {
	case GENE_TYPE_PROTEIN:
		metaGenes.push_back(&data.gene_protein.maxDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_protein.maxDepth.changeAmount);
		metaGenes.push_back(&data.gene_protein.minDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_protein.minDepth.changeAmount);
		metaGenes.push_back(&data.gene_protein.protein.chanceToMutate);
		metaGenes.push_back(&data.gene_protein.protein.changeAmount);
		metaGenes.push_back(&data.gene_protein.targetSegment.chanceToMutate);
		metaGenes.push_back(&data.gene_protein.targetSegment.changeAmount);
		break;
	case GENE_TYPE_OFFSET:
		metaGenes.push_back(&data.gene_offset.maxDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_offset.maxDepth.changeAmount);
		metaGenes.push_back(&data.gene_offset.minDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_offset.minDepth.changeAmount);
		metaGenes.push_back(&data.gene_offset.offset.chanceToMutate);
		metaGenes.push_back(&data.gene_offset.offset.changeAmount);
		metaGenes.push_back(&data.gene_offset.targetSegment.chanceToMutate);
		metaGenes.push_back(&data.gene_offset.targetSegment.changeAmount);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		metaGenes.push_back(&data.gene_attribute.maxDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_attribute.maxDepth.changeAmount);
		metaGenes.push_back(&data.gene_attribute.minDepth.chanceToMutate);
		metaGenes.push_back(&data.gene_attribute.minDepth.changeAmount);
		metaGenes.push_back(&data.gene_attribute.value.chanceToMutate);
		metaGenes.push_back(&data.gene_attribute.value.changeAmount);
		break;
	case GENE_TYPE_SYNAPSE:
		metaGenes.push_back(&data.gene_synapse.from.chanceToMutate);
		metaGenes.push_back(&data.gene_synapse.from.changeAmount);
		metaGenes.push_back(&data.gene_synapse.to.chanceToMutate);
		metaGenes.push_back(&data.gene_synapse.to.changeAmount);
		metaGenes.push_back(&data.gene_synapse.weight.chanceToMutate);
		metaGenes.push_back(&data.gene_synapse.weight.changeAmount);
		break;
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		metaGenes.push_back(&data.gene_feedback_synapse.from.chanceToMutate);
		metaGenes.push_back(&data.gene_feedback_synapse.from.changeAmount);
		metaGenes.push_back(&data.gene_feedback_synapse.to.chanceToMutate);
		metaGenes.push_back(&data.gene_feedback_synapse.to.changeAmount);
		metaGenes.push_back(&data.gene_feedback_synapse.weight.chanceToMutate);
		metaGenes.push_back(&data.gene_feedback_synapse.weight.changeAmount);
		break;
	case GENE_TYPE_TRANSFER:
		metaGenes.push_back(&data.gene_transfer_function.targetNeuron.chanceToMutate);
		metaGenes.push_back(&data.gene_transfer_function.targetNeuron.changeAmount);
		metaGenes.push_back(&data.gene_transfer_function.functionID.chanceToMutate);
		metaGenes.push_back(&data.gene_transfer_function.functionID.changeAmount);
		break;
	case GENE_TYPE_NEURAL_CONST:
		metaGenes.push_back(&data.gene_neural_constant.targetNeuron.chanceToMutate);
		metaGenes.push_back(&data.gene_neural_constant.targetNeuron.changeAmount);
		metaGenes.push_back(&data.gene_neural_constant.value.chanceToMutate);
		metaGenes.push_back(&data.gene_neural_constant.value.changeAmount);
		break;
	case GENE_TYPE_BODY_ATTRIBUTE:
		metaGenes.push_back(&data.gene_body_attribute.value.chanceToMutate);
		metaGenes.push_back(&data.gene_body_attribute.value.changeAmount);
		break;
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
	case GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO:
		g.value.set(BodyConst::initialFatMassRatio);
		break;
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
	GeneProtein g;
	g.maxDepth.set(randi(5));
	g.minDepth.set(0);
	g.protein.set((gene_protein_type)randi(GENE_PROT_NONE+1, GENE_PROT_END-1));
	g.targetSegment.set(randi(MAX_CHILDREN));
	return g;
}

Gene Gene::createRandomOffsetGene(int spaceLeftAfter) {
	GeneOffset g;
	g.maxDepth.set(randi(5));
	g.minDepth.set(0);
	g.targetSegment.set(randi(MAX_CHILDREN));
	g.offset.set(randi(spaceLeftAfter));
	return g;
}

Gene Gene::createRandomSynapseGene(int nNeurons, int nMotors, int nSensors) {
	GeneSynapse g;
	g.from.set(randi(-nSensors, nNeurons-1));
	g.to.set(randi(-nMotors, nNeurons-1));
	g.weight.set(randf());
	return g;
}

Gene Gene::createRandomFeedbackSynapseGene(int nMotors, int nNeurons) {
	GeneFeedbackSynapse g;
	g.from.set(randi(nMotors-1));
	g.from.set(randi(-nMotors, nNeurons-1));
	g.weight.set(srandf());
	return g;
}

Gene Gene::createRandomNeuralConstGene(int nNeurons) {
	GeneNeuralConstant g;
	g.targetNeuron.set(randi(nNeurons-1));
	g.value.set(srandf());
	return g;
}

Gene Gene::createRandomTransferFuncGene(int nNeurons) {
	GeneTransferFunction g;
	g.functionID.set(randi((int)transferFuncNames::FN_MAXCOUNT-1));
	g.targetNeuron.set(randi(nNeurons-1));
	return g;
}

Gene Gene::createRandomAttribGene() {
	GeneAttribute g;
	g.attribute = (gene_part_attribute_type)randi(GENE_ATTRIB_INVALID+1, GENE_ATTRIB_END-1);
	g.value.set(randf());
	return g;
}

Gene Gene::createRandomSkipGene(int spaceLeftAfter) {
	GeneSkip g;
	g.minDepth.set(randi(10));
	g.maxDepth.set(g.minDepth + randi(10-g.minDepth));
	g.count.set(randi(spaceLeftAfter));
	return g;
}

Gene Gene::createRandom(int spaceLeftAfter, int nMotors, int nSensors, int nNeurons) {
	gene_type type = (gene_type)randi(GENE_TYPE_INVALID+1, GENE_TYPE_END-1);
	switch (type) {
	case GENE_TYPE_BODY_ATTRIBUTE:
		return createRandomBodyAttribGene();
	case GENE_TYPE_PROTEIN:
		return createRandomProteinGene();
	case GENE_TYPE_OFFSET:
		return createRandomOffsetGene(spaceLeftAfter);
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		return createRandomFeedbackSynapseGene(nMotors, nNeurons);
	case GENE_TYPE_NEURAL_CONST:
		return createRandomNeuralConstGene(nNeurons);
	case GENE_TYPE_PART_ATTRIBUTE:
		return createRandomAttribGene();
	case GENE_TYPE_SKIP:
		return createRandomSkipGene(spaceLeftAfter);
	case GENE_TYPE_STOP:
		return GeneStop();
	case GENE_TYPE_SYNAPSE:
		return createRandomSynapseGene(nNeurons, nMotors, nSensors);
	case GENE_TYPE_TRANSFER:
		return createRandomTransferFuncGene(nNeurons);
	case GENE_TYPE_NO_OP:
		return GeneNoOp();
	default:
		ERROR("unhandled gene random type: " << type);
		return GeneStop();
	}
}
