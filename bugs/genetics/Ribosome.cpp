#include "Ribosome.h"

#include "../neuralnet/functions.h"
#include "../math/tools.h"
#include "../log.h"
#include "../neuralnet/Network.h"
#include "../neuralnet/Neuron.h"
#include "../neuralnet/OutputSocket.h"
#include "Gene.h"
#include "GeneDefinitions.h"
#include "DevelopmentNode.h"
#include "../entities/Bug.h"
#include "../objects/body-parts/Torso.h"
#include "../objects/body-parts/Bone.h"
#include "../objects/body-parts/Gripper.h"
#include "../objects/body-parts/Joint.h"
#include "../objects/body-parts/ZygoteShell.h"
#include "../log.h"

#include "Genome.h"
using namespace std;

Ribosome::Ribosome(Bug* bug)
	: bug{bug}
	, crtPosition{0}
	, root(nullptr)
{
}

bool Ribosome::step() {
	bool hasFirst = crtPosition < bug->genome.first.size();
	bool hasSecond = crtPosition < bug->genome.second.size();
	if (!hasFirst && !hasSecond) {
		// decoding sequence finished
		delete root;
		return false;
	}
	Gene* g = nullptr;
	// choose the dominant (or the only) gene out of the current pair:
	if (hasFirst && (!hasSecond || bug->genome.first[crtPosition].RID > bug->genome.second[crtPosition].RID))
		g = &bug->genome.first[crtPosition];
	else
		g = &bug->genome.second[crtPosition];

	// now decode the gene
	switch (g->type) {
	case GENE_TYPE_DEVELOPMENT:
		decodeGrowth(g->data.gene_command);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		decodePartAttrib(g->data.gene_local_attribute);
		break;
	case GENE_TYPE_GENERAL_ATTRIB:
		decodeGeneralAttrib(g->data.gene_general_attribute);
		break;
	case GENE_TYPE_NEURON:
		// add new neuron here
		break;
	case GENE_TYPE_SYNAPSE:
		decodeSynapse(g->data.gene_synapse);
		break;
	case GENE_TYPE_TRANSFER:
		decodeTransferFn(g->data.gene_transfer_function);
		break;
	case GENE_TYPE_MUSCLE_COMMAND:
		decodeMuscleCommand(g->data.gene_muscle_command);
		break;
	default:
		LOG("Invalid gene type : " << g->type);
	}

	// move to next position
	crtPosition++;
	return true;
}

bool Ribosome::partMustGenerateJoint(int part_type) {
	switch (part_type) {
	case BODY_PART_BONE:
	case BODY_PART_GRIPPER:
		return true;
	default:
		return false;
	}
}

void Ribosome::decodeGrowth(GeneCommand const& g) {
	std::list<DevelopmentNode*> nodes;
	root->matchLocation(g.location, &nodes);

	if (g.command == GENE_DEV_GROW) {
		// now grow a new part on each adequate element in nodes list
		for (auto n : nodes) {
			// grow only works on bones and torso
			if (n->bodyPart->getType() != BODY_PART_BONE && n->bodyPart->getType() != BODY_PART_TORSO)
				continue;
			if (n->nChildren == 4)
				continue;
			if (partMustGenerateJoint(g.part_type)) {
				// we cannot grow this part directly onto its parent, they must be connected by a joint
				glm::vec2 offset(0, 0);
				Joint* linkJoint = new Joint(n->bodyPart, PhysicsProperties(offset, 0));
				n->children[n->nChildren++] = new DevelopmentNode(n, linkJoint);
				// set n to point to the joint's node, since that's where the actual part will be attached:
				n = n->children[n->nChildren-1];
			}
			glm::vec2 offset(0);
			float angle = g.angle;
			// must compute the position and angle of the new part
			//...

			BodyPart* bp = nullptr;
			switch (g.part_type) {
			case BODY_PART_BONE:
				bp = new Bone(n->bodyPart, PhysicsProperties(offset, angle));
				break;
			case BODY_PART_GRIPPER:
				bp = new Gripper(n->bodyPart, PhysicsProperties(offset, angle));
				break;
			case BODY_PART_MUSCLE:
				// bp = new Muscle(n->bodyPart, PhysicsProperties(offset, angle));
				break;
			case BODY_PART_SENSOR:
				// bp = new sensortype?(n->bodyPart, PhysicsProperties(offset, angle));
				break;
			default:
				break;
			}
			if (!bp)
				continue;
			n->children[n->nChildren++] = new DevelopmentNode(n, bp);
		}
	} else if (g.command == GENE_DEV_SPLIT) {
		// split may work on bones and joints only
		for (auto n : nodes) {
			if (n->bodyPart->getType() != BODY_PART_BONE && n->bodyPart->getType() != BODY_PART_JOINT)
				continue;
		}
	}
}

void Ribosome::decodePartAttrib(GeneLocalAttribute const& g) {

}

void Ribosome::decodeGeneralAttrib(GeneGeneralAttribute const& g) {

}

void Ribosome::decodeSynapse(GeneSynapse const& g) {

}

void Ribosome::decodeTransferFn(GeneTransferFunction const& g) {

}

void Ribosome::decodeMuscleCommand(GeneMuscleCommand const& g) {

}
