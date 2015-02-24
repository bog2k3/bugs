#include "Gene.h"
#include "../utils/log.h"
#include "../body-parts/BodyConst.h"

void Gene::update_meta_genes_vec() {
	metaGenes.clear();
	// add meta-genes to this vector to enable their mutation:
	metaGenes.push_back(&chance_to_delete);
	metaGenes.push_back(&chance_to_swap);

	switch (type) {
	case GENE_TYPE_DEVELOPMENT:
		metaGenes.push_back(&data.gene_command.angle.chanceToMutate);
		metaGenes.push_back(&data.gene_command.angle.changeAmount);
		metaGenes.push_back(&data.gene_command.genomeOffset.chanceToMutate);
		metaGenes.push_back(&data.gene_command.genomeOffset.changeAmount);
		metaGenes.push_back(&data.gene_command.genomeOffsetJoint.chanceToMutate);
		metaGenes.push_back(&data.gene_command.genomeOffsetJoint.changeAmount);
		metaGenes.push_back(&data.gene_command.genomeOffsetMuscle1.chanceToMutate);
		metaGenes.push_back(&data.gene_command.genomeOffsetMuscle1.changeAmount);
		metaGenes.push_back(&data.gene_command.genomeOffsetMuscle2.chanceToMutate);
		metaGenes.push_back(&data.gene_command.genomeOffsetMuscle2.changeAmount);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
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

Gene Gene::createRandom() {
	gene_type type = (gene_type)randi(GENE_TYPE_INVALID+1, GENE_TYPE_END-1);
	switch (type) {
	case GENE_TYPE_BODY_ATTRIBUTE:
		return createRandomBodyAttribGene();
	case GENE_TYPE_DEVELOPMENT:
		return createRandomCommandGene();
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		return createRandomFeedbackSynapseGene();
	case GENE_TYPE_NEURAL_CONST:
		return createRandomNeuralConstGene();
	case GENE_TYPE_PART_ATTRIBUTE:
		return createRandomAttribGene();
	case GENE_TYPE_SKIP:
		return createRandomSkipGene();
	case GENE_TYPE_STOP:
		return GeneStop();
	case GENE_TYPE_SYNAPSE:
		return createRandomSynapseGene();
	case GENE_TYPE_TRANSFER:
		return createRandomTransferFuncGene();
	default:
		ERROR("unhandled gene random type: " << type);
		return GeneStop();
	}
}
