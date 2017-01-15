/*
 * GeneSerialization.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#include "BinaryStream.h"
#include "../genetics/Gene.h"

BinaryStream& operator << (BinaryStream &stream, MetaGene const& mg) {
	stream << mg.dynamic_variation << mg.value;
	return stream;
}

BinaryStream& operator >> (BinaryStream &stream, MetaGene &mg) {
	stream >> mg.dynamic_variation >> mg.value;
	return stream;
}

template <typename T>
BinaryStream& operator << (BinaryStream &stream, Atom<T> const& atom) {
	stream << atom.value << atom.chanceToMutate << atom.changeAmount;
	return stream;
}

template <typename T>
BinaryStream& operator >> (BinaryStream &stream, Atom<T> &atom) {
	T val;
	stream >> val >> atom.chanceToMutate >> atom.changeAmount;
	atom.value = val;
	return stream;
}

BinaryStream& operator << (BinaryStream &stream, gene_type type) {
	stream << static_cast<int>(type);
	return stream;
}

BinaryStream& operator >> (BinaryStream &stream, gene_type &type) {
	int itype;
	stream >> itype;
	type = static_cast<gene_type>(itype);
	return stream;
}

BinaryStream& operator << (BinaryStream &stream, Gene const& gene) {
	stream << gene.type;
	stream << gene.chance_to_delete;
	stream << gene.chance_to_swap;
	switch (gene.type) {
	case gene_type::BODY_ATTRIBUTE:
		stream << gene.data.gene_body_attribute.attribute;
		stream << gene.data.gene_body_attribute.value;
		break;
	case gene_type::PROTEIN:
		stream << gene.data.gene_protein.maxDepth;
		stream << gene.data.gene_protein.minDepth;
		stream << gene.data.gene_protein.protein;
		stream << gene.data.gene_protein.targetSegment;
		break;
	case gene_type::OFFSET:
		stream << gene.data.gene_offset.maxDepth;
		stream << gene.data.gene_offset.minDepth;
		stream << gene.data.gene_offset.offset;
		stream << gene.data.gene_offset.targetSegment;
		break;
	case gene_type::NEURON_INPUT_COORD:
		stream << gene.data.gene_neuron_input.destNeuronVirtIndex;
		stream << gene.data.gene_neuron_input.inCoord;
		break;
	case gene_type::NEURON_OUTPUT_COORD:
		stream << gene.data.gene_neuron_output.srcNeuronVirtIndex;
		stream << gene.data.gene_neuron_output.outCoord;
		break;
	case gene_type::NEURAL_BIAS:
		stream << gene.data.gene_neural_constant.targetNeuron;
		stream << gene.data.gene_neural_constant.value;
		break;
	case gene_type::NEURAL_PARAM:
		stream << gene.data.gene_neural_param.targetNeuron;
		stream << gene.data.gene_neural_param.value;
		break;
	case gene_type::PART_ATTRIBUTE:
		stream << gene.data.gene_attribute.attribute;
		stream << gene.data.gene_attribute.value;
		break;
	case gene_type::SKIP:
		stream << gene.data.gene_skip.count;
		stream << gene.data.gene_skip.maxDepth;
		stream << gene.data.gene_skip.minDepth;
		break;
	case gene_type::SYNAPSE:
		stream << gene.data.gene_synapse.from;
		stream << gene.data.gene_synapse.to;
		stream << gene.data.gene_synapse.weight;
		break;
	case gene_type::TRANSFER_FUNC:
		stream << gene.data.gene_transfer_function.functionID;
		stream << gene.data.gene_transfer_function.targetNeuron;
		break;
	case gene_type::JOINT_OFFSET:
		stream << gene.data.gene_joint_offset.maxDepth;
		stream << gene.data.gene_joint_offset.minDepth;
		stream << gene.data.gene_joint_offset.offset;
		break;
	case gene_type::NO_OP:
		break;
	case gene_type::STOP:
		break;
	case gene_type::START_MARKER:
		break;
	default:
		assert(false); // unknown gene type
	}
	return stream;
}

BinaryStream& operator >> (BinaryStream &stream, Gene &gene) {
	stream >> gene.type;
	stream >> gene.chance_to_delete;
	stream >> gene.chance_to_swap;
	switch (gene.type) {
	case gene_type::BODY_ATTRIBUTE:
		stream >> gene.data.gene_body_attribute.attribute;
		stream >> gene.data.gene_body_attribute.value;
		break;
	case gene_type::PROTEIN:
		stream >> gene.data.gene_protein.maxDepth;
		stream >> gene.data.gene_protein.minDepth;
		stream >> gene.data.gene_protein.protein;
		stream >> gene.data.gene_protein.targetSegment;
		break;
	case gene_type::OFFSET:
		stream >> gene.data.gene_offset.maxDepth;
		stream >> gene.data.gene_offset.minDepth;
		stream >> gene.data.gene_offset.offset;
		stream >> gene.data.gene_offset.targetSegment;
		break;
	case gene_type::NEURON_INPUT_COORD:
		stream >> gene.data.gene_neuron_input.destNeuronVirtIndex;
		stream >> gene.data.gene_neuron_input.inCoord;
		break;
	case gene_type::NEURON_OUTPUT_COORD:
		stream >> gene.data.gene_neuron_output.srcNeuronVirtIndex;
		stream >> gene.data.gene_neuron_output.outCoord;
		break;
	case gene_type::NEURAL_BIAS:
		stream >> gene.data.gene_neural_constant.targetNeuron;
		stream >> gene.data.gene_neural_constant.value;
		break;
	case gene_type::NEURAL_PARAM:
		stream >> gene.data.gene_neural_param.targetNeuron;
		stream >> gene.data.gene_neural_param.value;
		break;
	case gene_type::PART_ATTRIBUTE:
		stream >> gene.data.gene_attribute.attribute;
		stream >> gene.data.gene_attribute.value;
		break;
	case gene_type::SKIP:
		stream >> gene.data.gene_skip.count;
		stream >> gene.data.gene_skip.maxDepth;
		stream >> gene.data.gene_skip.minDepth;
		break;
	case gene_type::SYNAPSE:
		stream >> gene.data.gene_synapse.from;
		stream >> gene.data.gene_synapse.to;
		stream >> gene.data.gene_synapse.weight;
		break;
	case gene_type::TRANSFER_FUNC:
		stream >> gene.data.gene_transfer_function.functionID;
		stream >> gene.data.gene_transfer_function.targetNeuron;
		break;
	case gene_type::JOINT_OFFSET:
		stream >> gene.data.gene_joint_offset.maxDepth;
		stream >> gene.data.gene_joint_offset.minDepth;
		stream >> gene.data.gene_joint_offset.offset;
		break;
	case gene_type::NO_OP:
		break;
	case gene_type::STOP:
		break;
	case gene_type::START_MARKER:
		break;
	default:
		assert(false); // unknown gene type
	}
	return stream;
}
