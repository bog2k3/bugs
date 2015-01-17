#include "Gene.h"

void Gene::update_meta_genes_vec() {
	metaGenes.clear();
	// add meta-genes to this vector to enable their segregation:
	metaGenes.push_back(&chance_to_delete);
	metaGenes.push_back(&chance_to_swap);

#warning "update all gene types"

	switch (type) {
	case GENE_TYPE_DEVELOPMENT:
		metaGenes.push_back(&data.gene_command.angle.meta);
		break;
	case GENE_TYPE_GENERAL_ATTRIB:
		metaGenes.push_back(&data.gene_general_attribute.value.meta);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		metaGenes.push_back(&data.gene_local_attribute.value.meta);
		break;
	case GENE_TYPE_SYNAPSE:
		metaGenes.push_back(&data.gene_synapse.from.meta);
		metaGenes.push_back(&data.gene_synapse.to.meta);
		metaGenes.push_back(&data.gene_synapse.weight.meta);
		break;
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		metaGenes.push_back(&data.gene_feedback_synapse.from.meta);
		metaGenes.push_back(&data.gene_feedback_synapse.to.meta);
		metaGenes.push_back(&data.gene_feedback_synapse.weight.meta);
		break;
	case GENE_TYPE_TRANSFER:
		metaGenes.push_back(&data.gene_transfer_function.targetNeuron.meta);
		metaGenes.push_back(&data.gene_transfer_function.functionID.meta);
		break;
	case GENE_TYPE_BODY_ATTRIBUTE:
		metaGenes.push_back(&data.gene_body_attribute.value.meta);
		break;
	default:
		break;
	}
}
