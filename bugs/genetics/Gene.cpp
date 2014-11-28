#include "Gene.h"

std::map<gene_type, gene_value_type> Gene::mapGeneValueTypes;

int populateMap() {
	Gene::mapGeneValueTypes[GENE_INPUT_SOURCE] = GENE_VALUE_INT;
	Gene::mapGeneValueTypes[GENE_INPUT_WEIGHT] = GENE_VALUE_DOUBLE;
	Gene::mapGeneValueTypes[GENE_BIAS] = GENE_VALUE_DOUBLE;
	Gene::mapGeneValueTypes[GENE_TRANSFER_FUNCTION] = GENE_VALUE_INT;
	Gene::mapGeneValueTypes[GENE_TRANSFER_ARGUMENT] = GENE_VALUE_DOUBLE;
	Gene::mapGeneValueTypes[GENE_OUTPUT] = GENE_VALUE_DOUBLE;
	return 0;
}

int dummyToPopulateMap = populateMap();
