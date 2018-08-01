/*
 * GeneSerialization.h
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#ifndef SERIALIZATION_GENESERIALIZATION_H_
#define SERIALIZATION_GENESERIALIZATION_H_

#include "../genetics/Gene.h"
#include "../neuralnet/functions.h"

template<class StreamType>
StreamType& operator << (StreamType &stream, MetaGene const& mg) {
	stream << mg.dynamic_variation << mg.value;
	return stream;
}

template<class StreamType>
StreamType& operator >> (StreamType &stream, MetaGene &mg) {
	stream >> mg.dynamic_variation >> mg.value;
	if (std::isnan(mg.value))
		mg.value = constants::initial_gene_mutate;
	if (std::isnan(mg.dynamic_variation))
		mg.dynamic_variation = constants::change_gene_mutate;
	return stream;
}

template <class StreamType, class T>
StreamType& operator << (StreamType &stream, Atom<T> const& atom) {
	stream << atom.value << atom.chanceToMutate << atom.changeAmount;
	return stream;
}

template <class StreamType, class T>
StreamType& operator >> (StreamType &stream, Atom<T> &atom) {
	T val;
	stream >> val >> atom.chanceToMutate >> atom.changeAmount;
	if (std::isnan(val))
		val = constants::small_gene_value;
	atom.value = val;
	return stream;
}

template<class StreamType>
StreamType& operator << (StreamType &stream, gene_type type) {
	stream << static_cast<int>(type);
	return stream;
}

template<class StreamType>
StreamType& operator >> (StreamType &stream, gene_type &type) {
	int itype;
	stream >> itype;
	itype = clamp(itype, (int)gene_type::INVALID + 1, (int)gene_type::END - 1);
	type = static_cast<gene_type>(itype);
	return stream;
}

template<class StreamType>
StreamType& operator << (StreamType &stream, BranchRestriction const& br) {
	stream << br.activeLevels;
	uint totalLevels = sizeof(br.levels) / sizeof(br.levels[0]);
	stream << totalLevels;
	for (uint i=0; i<totalLevels; i++) {
		stream << br.levels[i].skipLeft;
		stream << br.levels[i].skipRight;
		stream << br.levels[i].stopLeft;
		stream << br.levels[i].stopRight;
	}
	return stream;
}

template<class StreamType>
StreamType& operator >> (StreamType &stream, BranchRestriction &br) {
	stream >> br.activeLevels;
	uint maxLevels = sizeof(br.levels) / sizeof(br.levels[0]);
	if (br.activeLevels > maxLevels)
		br.activeLevels.set(maxLevels);
	uint totalLevels;
	stream >> totalLevels;
	totalLevels = min(totalLevels, maxLevels);
	for (uint i=0; i<totalLevels; i++) {
		Atom<float> skipLeft, skipRight, stopLeft, stopRight;
		stream >> skipLeft >> skipRight >> stopLeft >> stopRight;
		if (i < maxLevels) {
			br.levels[i].skipLeft = skipLeft;
			br.levels[i].skipRight = skipRight;
			br.levels[i].stopLeft = stopLeft;
			br.levels[i].stopRight = stopRight;
		}
	}
	return stream;
}

template<class StreamType>
StreamType& operator << (StreamType &stream, Gene const& gene) {
	stream << gene.type;
	stream << gene.chance_to_delete;
	stream << gene.chance_to_swap;
	switch (gene.type) {
	case gene_type::BODY_ATTRIBUTE:
		stream << gene.data.gene_body_attribute.attribute;
		stream << gene.data.gene_body_attribute.value;
		break;
	case gene_type::DIVISION_PARAM:
		stream << gene.data.gene_division_param.param;
		stream << gene.data.gene_division_param.value;
		stream << gene.data.gene_division_param.restriction;
		break;
	case gene_type::JOINT_ATTRIBUTE:
		stream << gene.data.gene_joint_attrib.attrib;
		stream << gene.data.gene_joint_attrib.value;
		stream << gene.data.gene_joint_attrib.restriction;
		break;
	case gene_type::MUSCLE_ATTRIBUTE:
		stream << gene.data.gene_muscle_attrib.attrib;
		stream << gene.data.gene_muscle_attrib.side;
		stream << gene.data.gene_muscle_attrib.value;
		stream << gene.data.gene_muscle_attrib.restriction;
		break;
	case gene_type::NEURAL_BIAS:
		stream << gene.data.gene_neural_constant.neuronLocation;
		stream << gene.data.gene_neural_constant.value;
		stream << gene.data.gene_neural_constant.restriction;
		break;
	case gene_type::NEURAL_PARAM:
		stream << gene.data.gene_neural_param.neuronLocation;
		stream << gene.data.gene_neural_param.value;
		stream << gene.data.gene_neural_param.restriction;
		break;
	case gene_type::NEURON:
		stream << gene.data.gene_neuron.neuronLocation;
		stream << gene.data.gene_neuron.restriction;
		break;
	case gene_type::NO_OP:
		break;
	case gene_type::OFFSET:
		stream << gene.data.gene_offset.offset;
		stream << gene.data.gene_offset.side;
		stream << gene.data.gene_offset.restriction;
		break;
	case gene_type::PART_ATTRIBUTE:
		stream << gene.data.gene_attribute.attribute;
		stream << gene.data.gene_attribute.value;
		stream << gene.data.gene_attribute.restriction;
		break;
	case gene_type::PROTEIN:
		stream << gene.data.gene_protein.protein;
		stream << gene.data.gene_protein.weight;
		stream << gene.data.gene_protein.restriction;
		break;
	case gene_type::SKIP:
		stream << gene.data.gene_skip.count;
		stream << gene.data.gene_skip.restriction;
		break;
	case gene_type::STOP:
		break;
	case gene_type::SYNAPSE:
		stream << gene.data.gene_synapse.srcLocation;
		stream << gene.data.gene_synapse.destLocation;
		stream << gene.data.gene_synapse.weight;
		stream << gene.data.gene_synapse.priority;
		stream << gene.data.gene_synapse.restriction;
		break;
	case gene_type::TIME_SYNAPSE:
		stream << gene.data.gene_time_synapse.targetLocation;
		stream << gene.data.gene_time_synapse.weight;
		stream << gene.data.gene_time_synapse.restriction;
		break;
	case gene_type::TRANSFER_FUNC:
		stream << gene.data.gene_transfer_function.functionID;
		stream << gene.data.gene_transfer_function.neuronLocation;
		stream << gene.data.gene_transfer_function.restriction;
		break;
	case gene_type::VMS_OFFSET:
		stream << gene.data.gene_vms_offset.value;
		stream << gene.data.gene_vms_offset.restriction;
		break;
	default:
		assert(false); // unknown gene type
	}
	return stream;
}

template<class StreamType>
StreamType& operator >> (StreamType &stream, Gene &gene) {
	stream >> gene.type;
	stream >> gene.chance_to_delete;
	stream >> gene.chance_to_swap;
	switch (gene.type) {
	case gene_type::BODY_ATTRIBUTE:
		stream >> gene.data.gene_body_attribute.attribute;
		stream >> gene.data.gene_body_attribute.value;

		gene.data.gene_body_attribute.attribute = clamp((int)gene.data.gene_body_attribute.attribute,
				GENE_BODY_ATTRIB_INVALID + 1,
				GENE_BODY_ATTRIB_END - 1);
		break;
	case gene_type::DIVISION_PARAM:
		stream >> gene.data.gene_division_param.param;
		stream >> gene.data.gene_division_param.value;
		stream >> gene.data.gene_division_param.restriction;

		gene.data.gene_division_param.param = clamp((int)gene.data.gene_division_param.param,
				GENE_DIVISION_INVALID + 1,
				GENE_DIVISION_END - 1);
		break;
	case gene_type::JOINT_ATTRIBUTE:
		stream >> gene.data.gene_joint_attrib.attrib;
		stream >> gene.data.gene_joint_attrib.value;
		stream >> gene.data.gene_joint_attrib.restriction;

		gene.data.gene_joint_attrib.attrib = clamp((int)gene.data.gene_joint_attrib.attrib,
				GENE_JOINT_ATTR_INVALID + 1,
				GENE_JOINT_ATTR_END - 1);
		break;
	case gene_type::MUSCLE_ATTRIBUTE:
		stream >> gene.data.gene_muscle_attrib.attrib;
		stream >> gene.data.gene_muscle_attrib.side;
		stream >> gene.data.gene_muscle_attrib.value;
		stream >> gene.data.gene_muscle_attrib.restriction;

		gene.data.gene_muscle_attrib.attrib = clamp((int)gene.data.gene_muscle_attrib.attrib,
				GENE_MUSCLE_ATTR_INVALID + 1,
				GENE_MUSCLE_ATTR_END - 1);
		break;
	case gene_type::NEURAL_BIAS:
		stream >> gene.data.gene_neural_constant.neuronLocation;
		stream >> gene.data.gene_neural_constant.value;
		stream >> gene.data.gene_neural_constant.restriction;
		break;
	case gene_type::NEURAL_PARAM:
		stream >> gene.data.gene_neural_param.neuronLocation;
		stream >> gene.data.gene_neural_param.value;
		stream >> gene.data.gene_neural_param.restriction;
		break;
	case gene_type::NEURON:
		stream >> gene.data.gene_neuron.neuronLocation;
		stream >> gene.data.gene_neuron.restriction;
		break;
	case gene_type::NO_OP:
		break;
	case gene_type::OFFSET:
		stream >> gene.data.gene_offset.offset;
		stream >> gene.data.gene_offset.side;
		stream >> gene.data.gene_offset.restriction;
		break;
	case gene_type::PART_ATTRIBUTE:
		stream >> gene.data.gene_attribute.attribute;
		stream >> gene.data.gene_attribute.value;
		stream >> gene.data.gene_attribute.restriction;

		gene.data.gene_attribute.attribute = clamp((int)gene.data.gene_attribute.attribute,
				GENE_ATTRIB_INVALID + 1,
				GENE_ATTRIB_END - 1);
		break;
	case gene_type::PROTEIN:
		stream >> gene.data.gene_protein.protein;
		stream >> gene.data.gene_protein.weight;
		stream >> gene.data.gene_protein.restriction;

		gene.data.gene_protein.protein = clamp((int)gene.data.gene_protein.protein,
				GENE_PROT_INVALID + 1,
				GENE_PROT_END - 1);
		break;
	case gene_type::SKIP:
		stream >> gene.data.gene_skip.count;
		stream >> gene.data.gene_skip.restriction;
		break;
	case gene_type::STOP:
		break;
	case gene_type::SYNAPSE:
		stream >> gene.data.gene_synapse.srcLocation;
		stream >> gene.data.gene_synapse.destLocation;
		stream >> gene.data.gene_synapse.weight;
		stream >> gene.data.gene_synapse.priority;
		stream >> gene.data.gene_synapse.restriction;
		break;
	case gene_type::TIME_SYNAPSE:
		stream >> gene.data.gene_time_synapse.targetLocation;
		stream >> gene.data.gene_time_synapse.weight;
		stream >> gene.data.gene_time_synapse.restriction;
		break;
	case gene_type::TRANSFER_FUNC:
		stream >> gene.data.gene_transfer_function.functionID;
		stream >> gene.data.gene_transfer_function.neuronLocation;
		stream >> gene.data.gene_transfer_function.restriction;

		gene.data.gene_transfer_function.functionID.value = clamp(
				(int)gene.data.gene_transfer_function.functionID,
				(int)transferFuncNames::FN_ONE,
				(int)transferFuncNames::FN_MAXCOUNT - 1);
		break;
	case gene_type::VMS_OFFSET:
		stream >> gene.data.gene_vms_offset.value;
		stream >> gene.data.gene_vms_offset.restriction;
		break;
	default:
		assert(false); // unknown gene type
	}
	return stream;
}

#endif /* SERIALIZATION_GENESERIALIZATION_H_ */
