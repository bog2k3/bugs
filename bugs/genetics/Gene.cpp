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

#define ATOM(A) \
	metaGenes.push_back(&A.chanceToMutate); \
	metaGenes.push_back(&A.changeAmount);
#define RESTRICTION(R) \
	ATOM(R.activeLevels); \
	for (auto i=0u; i<sizeof(R.levels)/sizeof(BranchRestriction::levelRule); i++) { \
		ATOM(R.levels[i].skipLeft); \
		ATOM(R.levels[i].skipRight); \
		ATOM(R.levels[i].stopLeft); \
		ATOM(R.levels[i].stopRight); \
	}

	switch (type) {
	case gene_type::BODY_ATTRIBUTE:
		ATOM(data.gene_body_attribute.value)
		break;
	case gene_type::DIVISION_PARAM:
		ATOM(data.gene_division_param.value)
		RESTRICTION(data.gene_division_param.restriction)
		break;
	case gene_type::JOINT_ATTRIBUTE:
		ATOM(data.gene_joint_attrib.value);
		RESTRICTION(data.gene_joint_attrib.restriction)
		break;
	case gene_type::MUSCLE_ATTRIBUTE:
		ATOM(data.gene_muscle_attrib.side)
		ATOM(data.gene_muscle_attrib.value)
		RESTRICTION(data.gene_muscle_attrib.restriction)
		break;
	case gene_type::NEURAL_BIAS:
		ATOM(data.gene_neural_constant.neuronLocation)
		ATOM(data.gene_neural_constant.value)
		RESTRICTION(data.gene_neural_constant.restriction)
		break;
	case gene_type::NEURAL_PARAM:
		ATOM(data.gene_neural_param.neuronLocation)
		ATOM(data.gene_neural_param.value)
		RESTRICTION(data.gene_neural_param.restriction)
		break;
	case gene_type::NEURON:
		ATOM(data.gene_neuron.neuronLocation)
		RESTRICTION(data.gene_neuron.restriction)
		break;
	case gene_type::OFFSET:
		ATOM(data.gene_offset.offset)
		ATOM(data.gene_offset.side)
		RESTRICTION(data.gene_offset.restriction)
		break;
	case gene_type::PART_ATTRIBUTE:
		ATOM(data.gene_attribute.value)
		RESTRICTION(data.gene_attribute.restriction)
		break;
	case gene_type::PROTEIN:
		ATOM(data.gene_protein.weight);
		RESTRICTION(data.gene_protein.restriction);
		break;
	case gene_type::SKIP:
		ATOM(data.gene_skip.count)
		RESTRICTION(data.gene_skip.restriction)
		break;
	case gene_type::SYNAPSE:
		ATOM(data.gene_synapse.srcLocation)
		ATOM(data.gene_synapse.destLocation)
		ATOM(data.gene_synapse.priority)
		ATOM(data.gene_synapse.weight)
		RESTRICTION(data.gene_synapse.restriction)
		break;
	case gene_type::TIME_SYNAPSE:
		ATOM(data.gene_time_synapse.targetLocation)
		ATOM(data.gene_time_synapse.weight)
		RESTRICTION(data.gene_time_synapse.restriction)
		break;
	case gene_type::TRANSFER_FUNC:
		ATOM(data.gene_transfer_function.neuronLocation)
		ATOM(data.gene_transfer_function.functionID)
		RESTRICTION(data.gene_transfer_function.restriction)
		break;
	case gene_type::VMS_OFFSET:
		ATOM(data.gene_vms_offset.value)
		RESTRICTION(data.gene_vms_offset.restriction)
		break;
	case gene_type::STOP:
	case gene_type::NO_OP:
		break;
	default:
		throw std::runtime_error("You forgot to add a gene type!");
		break;
	}
#undef ATOM
#undef RESTRICTION
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
	case GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO:
		g.value.set(BodyConst::initialMinFatMassRatio);
		break;
	case GENE_BODY_ATTRIB_REPRODUCTIVE_MASS_RATIO:
		g.value.set(BodyConst::initialReproductiveMassRatio);
		break;
	case GENE_BODY_DEVELOPMENT_MASS_THRESH_RATIO:
		g.value.set(BodyConst::initialDevelopmentMassThreshRatio);
		break;
	default:
		throw std::runtime_error("unhandled body attrib type! ");
	}
	return g;
}

Gene Gene::createRandomProteinGene() {
	GeneProtein g;
	g.protein = (gene_protein_type)randi(GENE_PROT_INVALID+1, GENE_PROT_END-1);
	g.restriction.activeLevels.set(randi(constants::MAX_DIVISION_DEPTH));
	g.weight.set(srandf() * constants::small_gene_value);
	return g;
}

Gene Gene::createRandomOffsetGene(int spaceLeftAfter) {
	GeneOffset g;
	g.side.set(srandf() * constants::small_gene_value);
	g.restriction.activeLevels.set(randi(constants::MAX_DIVISION_DEPTH));
	g.offset.set(randi(spaceLeftAfter));
	return g;
}

Gene Gene::createRandomSynapseGene() {
	GeneSynapse g;
	g.srcLocation.set(srandf() * BodyConst::MaxVMSCoordinateValue);
	g.destLocation.set(srandf() * BodyConst::MaxVMSCoordinateValue);
	g.priority.set(randf()*10);
	g.weight.set(randf()*0.2f);
	return g;
}

Gene Gene::createRandomNeuralBiasGene() {
	GeneNeuralBias g;
	g.neuronLocation.set(srandf() * BodyConst::MaxVMSCoordinateValue);
	g.value.set(randf());
	return g;
}

Gene Gene::createRandomNeuralParamGene() {
	GeneNeuralParam g;
	g.neuronLocation.set(srandf() * BodyConst::MaxVMSCoordinateValue);
	g.value.set(randf());
	return g;
}

Gene Gene::createRandomTimeSynapse() {
	GeneTimeSynapse g;
	g.targetLocation.set(srandf() * BodyConst::MaxVMSCoordinateValue);
	g.weight.set(randf()*0.2f);
	return g;
}

Gene Gene::createRandomTransferFuncGene() {
	GeneTransferFunction g;
	g.neuronLocation.set(srandf() * BodyConst::MaxVMSCoordinateValue);
	g.functionID.set(randi((int)transferFuncNames::FN_MAXCOUNT-1));
	return g;
}

Gene Gene::createRandomAttribGene() {
	GeneAttribute g;
	g.attribute = (gene_part_attribute_type)randi(GENE_ATTRIB_INVALID+1, GENE_ATTRIB_END-1);
	g.value.set(randf());
	g.restriction.activeLevels.set(randi(constants::MAX_DIVISION_DEPTH));
	return g;
}

Gene Gene::createRandomSkipGene(int spaceLeftAfter) {
	GeneSkip g;
	g.restriction.activeLevels.set(randi(constants::MAX_DIVISION_DEPTH));
	// use a random distribution that favors small values:
	g.count.set(sqr(randf()) * spaceLeftAfter);
	return g;
}

Gene Gene::createRandomDivisionParamGene() {
	GeneDivisionParam g;
	g.restriction.activeLevels.set(randi(constants::MAX_DIVISION_DEPTH));
	g.param = (gene_division_param_type)randi(GENE_DIVISION_INVALID+1, GENE_DIVISION_END-1);
	g.value.set(srandf() * 2 * constants::small_gene_value);
	return g;
}

Gene Gene::createRandomVMSOffsetGene() {
	GeneVMSOffset g;
	g.restriction.activeLevels.set(randi(constants::MAX_DIVISION_DEPTH));
	g.value.set(srandf() * 50);
	return g;
}

Gene Gene::createRandomJointAttributeGene() {
	GeneJointAttribute g;
	g.attrib = (gene_joint_attribute_type)randi(GENE_JOINT_ATTR_INVALID+1, GENE_JOINT_ATTR_END-1);
	g.restriction.activeLevels.set(randi(constants::MAX_DIVISION_DEPTH));
	g.value.set(srandf());
	return g;
}

Gene Gene::createRandomMuscleAttributeGene() {
	GeneMuscleAttribute g;
	g.attrib = (gene_muscle_attribute_type)randi(GENE_MUSCLE_ATTR_INVALID+1, GENE_MUSCLE_ATTR_END-1);
	g.restriction.activeLevels.set(randi(constants::MAX_DIVISION_DEPTH));
	g.value.set(srandf());
	g.side.set(srandf() * constants::small_gene_value);
	return g;
}

Gene Gene::createRandomNeuronGene() {
	GeneNeuron g;
	g.neuronLocation.set(srandf() * BodyConst::MaxVMSCoordinateValue);
	return g;
}

Gene Gene::createRandom(int spaceLeftAfter) {
	std::vector<std::pair<gene_type, double>> geneChances {
		// these are relative chances:
		{gene_type::BODY_ATTRIBUTE, 1.0},
		{gene_type::DIVISION_PARAM, 1.9},
		{gene_type::JOINT_ATTRIBUTE, 0.5},
		{gene_type::MUSCLE_ATTRIBUTE, 0.9},
		{gene_type::NEURAL_BIAS, 1.0},
		{gene_type::NEURAL_PARAM, 0.8},
		{gene_type::NEURON, 1.2},
		{gene_type::NO_OP, 0.09},
		{gene_type::OFFSET, 0.3},
		{gene_type::PART_ATTRIBUTE, 2.1},
		{gene_type::PROTEIN, 1.5},
		{gene_type::SKIP, 0.1},
		{gene_type::STOP, 0.09},
		{gene_type::SYNAPSE, 1.5},
		{gene_type::TIME_SYNAPSE, 0.25},
		{gene_type::TRANSFER_FUNC, 0.5},
		{gene_type::VMS_OFFSET, 0.5},
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
	case gene_type::DIVISION_PARAM:
		return createRandomDivisionParamGene();
	case gene_type::JOINT_ATTRIBUTE:
		return createRandomJointAttributeGene();
	case gene_type::MUSCLE_ATTRIBUTE:
		return createRandomMuscleAttributeGene();
	case gene_type::NEURAL_BIAS:
		return createRandomNeuralBiasGene();
	case gene_type::NEURAL_PARAM:
		return createRandomNeuralParamGene();
	case gene_type::NEURON:
		return createRandomNeuronGene();
	case gene_type::NO_OP:
		return GeneNoOp();
	case gene_type::OFFSET:
		return createRandomOffsetGene(spaceLeftAfter);
	case gene_type::PART_ATTRIBUTE:
		return createRandomAttribGene();
	case gene_type::PROTEIN:
		return createRandomProteinGene();
	case gene_type::SKIP:
		return createRandomSkipGene(spaceLeftAfter);
	case gene_type::STOP:
		return GeneStop();
	case gene_type::SYNAPSE:
		return createRandomSynapseGene();
	case gene_type::TIME_SYNAPSE:
		return createRandomTimeSynapse();
	case gene_type::TRANSFER_FUNC:
		return createRandomTransferFuncGene();
	case gene_type::VMS_OFFSET:
		return createRandomVMSOffsetGene();
	default:
		throw std::runtime_error("You forgot to treat one gene type!");
	}
}

char Gene::getSymbol() const {
	switch (type) {
	case gene_type::BODY_ATTRIBUTE:
		return 'B';
	case gene_type::JOINT_ATTRIBUTE:
		return 'J';
	case gene_type::MUSCLE_ATTRIBUTE:
		return 'M';
	case gene_type::NEURAL_BIAS:
		return 'C';
	case gene_type::NEURON:
		return '*';
	case gene_type::NEURAL_PARAM:
		return 'N';
	case gene_type::TIME_SYNAPSE:
		return '$';
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
	case gene_type::STOP:
		return '!';
	case gene_type::SYNAPSE:
		return '~';
	case gene_type::TRANSFER_FUNC:
		return 'T';
	case gene_type::DIVISION_PARAM:
		return 'D';
	case gene_type::VMS_OFFSET:
		return 'V';
	default:
		throw std::runtime_error("Unhandled gene type!");
	}
}

bool Gene::operator==(Gene const& g) const {
	if (type != g.type)
		return false;
	if (chance_to_delete != g.chance_to_delete)
		return false;
	if (chance_to_swap != g.chance_to_swap)
		return false;
	switch (type) {
	case gene_type::BODY_ATTRIBUTE:
		if (data.gene_body_attribute.attribute != g.data.gene_body_attribute.attribute)
			return false;
		if (data.gene_body_attribute.value != g.data.gene_body_attribute.value)
			return false;
		break;
	case gene_type::JOINT_ATTRIBUTE:
		if (data.gene_joint_attrib.attrib != g.data.gene_joint_attrib.attrib)
			return false;
		if (data.gene_joint_attrib.restriction != g.data.gene_joint_attrib.restriction)
			return false;
		if (data.gene_joint_attrib.value != g.data.gene_joint_attrib.value)
			return false;
		break;
	case gene_type::MUSCLE_ATTRIBUTE:
		if (data.gene_muscle_attrib.attrib != g.data.gene_muscle_attrib.attrib)
			return false;
		if (data.gene_muscle_attrib.restriction != g.data.gene_muscle_attrib.restriction)
			return false;
		if (data.gene_muscle_attrib.side != g.data.gene_muscle_attrib.side)
			return false;
		if (data.gene_muscle_attrib.value != g.data.gene_muscle_attrib.value)
			return false;
		break;
	case gene_type::NEURAL_BIAS:
		if (data.gene_neural_constant.neuronLocation != g.data.gene_neural_constant.neuronLocation)
			return false;
		if (data.gene_neural_constant.restriction != g.data.gene_neural_constant.restriction)
			return false;
		if (data.gene_neural_constant.value != g.data.gene_neural_constant.value)
			return false;
		break;
	case gene_type::NEURON:
		if (data.gene_neuron.neuronLocation != g.data.gene_neuron.neuronLocation)
			return false;
		if (data.gene_neuron.restriction != g.data.gene_neuron.restriction)
			return false;
		break;
	case gene_type::NEURAL_PARAM:
		if (data.gene_neural_param.neuronLocation != g.data.gene_neural_param.neuronLocation)
			return false;
		if (data.gene_neural_param.restriction != g.data.gene_neural_param.restriction)
			return false;
		if (data.gene_neural_param.value != g.data.gene_neural_param.value)
			return false;
		break;
	case gene_type::TIME_SYNAPSE:
		if (data.gene_time_synapse.restriction != g.data.gene_time_synapse.restriction)
			return false;
		if (data.gene_time_synapse.targetLocation != g.data.gene_time_synapse.targetLocation)
			return false;
		if (data.gene_time_synapse.weight != g.data.gene_time_synapse.weight)
			return false;
		break;
	case gene_type::NO_OP:
		break;
	case gene_type::OFFSET:
		if (data.gene_offset.offset != g.data.gene_offset.offset)
			return false;
		if (data.gene_offset.restriction != g.data.gene_offset.restriction)
			return false;
		if (data.gene_offset.side != g.data.gene_offset.side)
			return false;
		break;
	case gene_type::PART_ATTRIBUTE:
		if (data.gene_attribute.attribute != g.data.gene_attribute.attribute)
			return false;
		if (data.gene_attribute.restriction != g.data.gene_attribute.restriction)
			return false;
		if (data.gene_attribute.value != g.data.gene_attribute.value)
			return false;
		break;
	case gene_type::PROTEIN:
		if (data.gene_protein.protein != g.data.gene_protein.protein)
			return false;
		if (data.gene_protein.restriction != g.data.gene_protein.restriction)
			return false;
		if (data.gene_protein.weight != g.data.gene_protein.weight)
			return false;
		break;
	case gene_type::SKIP:
		if (data.gene_skip.count != g.data.gene_skip.count)
			return false;
		if (data.gene_skip.restriction != g.data.gene_skip.restriction)
			return false;
		break;
	case gene_type::STOP:
		break;
	case gene_type::SYNAPSE:
		if (data.gene_synapse.destLocation != g.data.gene_synapse.destLocation)
			return false;
		if (data.gene_synapse.priority != g.data.gene_synapse.priority)
			return false;
		if (data.gene_synapse.restriction != g.data.gene_synapse.restriction)
			return false;
		if (data.gene_synapse.srcLocation != g.data.gene_synapse.srcLocation)
			return false;
		if (data.gene_synapse.weight != g.data.gene_synapse.weight)
			return false;
		break;
	case gene_type::TRANSFER_FUNC:
		if (data.gene_transfer_function.functionID != g.data.gene_transfer_function.functionID)
			return false;
		if (data.gene_transfer_function.neuronLocation != g.data.gene_transfer_function.neuronLocation)
			return false;
		if (data.gene_transfer_function.restriction != g.data.gene_transfer_function.restriction)
			return false;
		break;
	case gene_type::DIVISION_PARAM:
		if (data.gene_division_param.param != g.data.gene_division_param.param)
			return false;
		if (data.gene_division_param.restriction != g.data.gene_division_param.restriction)
			return false;
		if (data.gene_division_param.value != g.data.gene_division_param.value)
			return false;
		break;
	case gene_type::VMS_OFFSET:
		if (data.gene_vms_offset.restriction != g.data.gene_vms_offset.restriction)
			return false;
		if (data.gene_vms_offset.value != g.data.gene_vms_offset.value)
			return false;
		break;
	default:
		throw std::runtime_error("Unhandled gene type!");
	}
	return true;
}

bool BranchRestriction::operator==(BranchRestriction const& b) const {
	if (activeLevels != b.activeLevels)
		return false;
	for (unsigned i=0; i<activeLevels; i++) {
		if (levels[i].skipLeft != b.levels[i].skipLeft)
			return false;
		if (levels[i].skipRight!= b.levels[i].skipRight)
			return false;
		if (levels[i].stopLeft != b.levels[i].stopLeft)
			return false;
		if (levels[i].stopRight != b.levels[i].stopRight)
			return false;
	}
	return true;
}
