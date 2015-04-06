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

BinaryStream& operator << (BinaryStream &stream, Gene const& gene) {
	stream << gene.RID << gene.type;
	stream << gene.chance_to_delete;
	stream << gene.chance_to_swap;
	switch (gene.type) {
	case GENE_TYPE_BODY_ATTRIBUTE:
		stream << gene.data.gene_body_attribute.attribute;
		stream << gene.data.gene_body_attribute.value;
		break;
	case GENE_TYPE_DEVELOPMENT:
		stream << gene.data.gene_command.command;
		stream << gene.data.gene_command.angle;
		stream << gene.data.gene_command.age;
		stream << gene.data.gene_command.maxDepth;
		stream << gene.data.gene_command.part_type;
		stream << gene.data.gene_command.genomeOffset;
		stream << gene.data.gene_command.genomeOffsetJoint;
		stream << gene.data.gene_command.genomeOffsetMuscle1;
		stream << gene.data.gene_command.genomeOffsetMuscle2;
		break;
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		stream << gene.data.gene_feedback_synapse.from;
		stream << gene.data.gene_feedback_synapse.to;
		stream << gene.data.gene_feedback_synapse.weight;
		break;
	case GENE_TYPE_NEURAL_CONST:
		stream << gene.data.gene_neural_constant.targetNeuron;
		stream << gene.data.gene_neural_constant.value;
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		stream << gene.data.gene_attribute.attribute;
		stream << gene.data.gene_attribute.value;
		break;
	case GENE_TYPE_SKIP:
		stream << gene.data.gene_skip.count;
		stream << gene.data.gene_skip.maxDepth;
		stream << gene.data.gene_skip.minDepth;
		break;
	case GENE_TYPE_SYNAPSE:
		stream << gene.data.gene_synapse.from;
		stream << gene.data.gene_synapse.to;
		stream << gene.data.gene_synapse.weight;
		break;
	case GENE_TYPE_TRANSFER:
		stream << gene.data.gene_transfer_function.functionID;
		stream << gene.data.gene_transfer_function.targetNeuron;
		break;
	case GENE_TYPE_NO_OP:
		break;
	case GENE_TYPE_STOP:
		break;
	default:
		assert(false); // unknown gene type
	}
	return stream;
}

BinaryStream& operator >> (BinaryStream &stream, Gene &gene) {
	stream >> gene.RID >> gene.type;
	stream >> gene.chance_to_delete;
	stream >> gene.chance_to_swap;
	switch (gene.type) {
	case GENE_TYPE_BODY_ATTRIBUTE:
		stream >> gene.data.gene_body_attribute.attribute;
		stream >> gene.data.gene_body_attribute.value;
		break;
	case GENE_TYPE_DEVELOPMENT:
		stream >> gene.data.gene_command.command;
		stream >> gene.data.gene_command.angle;
		stream >> gene.data.gene_command.age;
		stream >> gene.data.gene_command.maxDepth;
		stream >> gene.data.gene_command.part_type;
		stream >> gene.data.gene_command.genomeOffset;
		stream >> gene.data.gene_command.genomeOffsetJoint;
		stream >> gene.data.gene_command.genomeOffsetMuscle1;
		stream >> gene.data.gene_command.genomeOffsetMuscle2;
		break;
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		stream >> gene.data.gene_feedback_synapse.from;
		stream >> gene.data.gene_feedback_synapse.to;
		stream >> gene.data.gene_feedback_synapse.weight;
		break;
	case GENE_TYPE_NEURAL_CONST:
		stream >> gene.data.gene_neural_constant.targetNeuron;
		stream >> gene.data.gene_neural_constant.value;
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		stream >> gene.data.gene_attribute.attribute;
		stream >> gene.data.gene_attribute.value;
		break;
	case GENE_TYPE_SKIP:
		stream >> gene.data.gene_skip.count;
		stream >> gene.data.gene_skip.maxDepth;
		stream >> gene.data.gene_skip.minDepth;
		break;
	case GENE_TYPE_SYNAPSE:
		stream >> gene.data.gene_synapse.from;
		stream >> gene.data.gene_synapse.to;
		stream >> gene.data.gene_synapse.weight;
		break;
	case GENE_TYPE_TRANSFER:
		stream >> gene.data.gene_transfer_function.functionID;
		stream >> gene.data.gene_transfer_function.targetNeuron;
		break;
	case GENE_TYPE_NO_OP:
		break;
	case GENE_TYPE_STOP:
		break;
	default:
		assert(false); // unknown gene type
	}
	return stream;
}