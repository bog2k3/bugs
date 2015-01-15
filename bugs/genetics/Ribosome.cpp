#include "Ribosome.h"

#include "../neuralnet/functions.h"
#include "../math/tools.h"
#include "../math/math2D.h"
#include "../log.h"
#include "../neuralnet/Network.h"
#include "../neuralnet/Neuron.h"
#include "../neuralnet/OutputSocket.h"
#include "Gene.h"
#include "GeneDefinitions.h"
#include "CummulativeValue.h"
#include "../entities/Bug.h"
#include "../objects/body-parts/BodyPart.h"
#include "../objects/body-parts/Torso.h"
#include "../objects/body-parts/Bone.h"
#include "../objects/body-parts/Gripper.h"
#include "../objects/body-parts/Joint.h"
#include "../objects/body-parts/ZygoteShell.h"
#include "../objects/body-parts/Muscle.h"
#include "../log.h"

#include "Genome.h"
using namespace std;

static constexpr float MUSCLE_OFFSET_ANGLE = PI * 0.25f;

Ribosome::Ribosome(Bug* bug)
	: bug_{bug}
	, crtPosition_{0}
	, root_{bug_->body_}
{
}

// compares two unsigned longs as if they were expressed as coordinates in a circular scale
// (where the largest number comes right before 0 and is considered smaller than 0)
// X1 is greater than X2 on this scale if X1 is to the left of X2 (the coordinates grow in
// geometrical direction - counter-clockwise). That means the path from X1 back to X2 is shorter
// than the path from X1 forward to X2.
// This model guarantees that any number has half other numbers greater than it and half smaller than it,
// which is a needed condition for gene dominance, so that no number is privileged in an absolute manner,
// only relative to other genes.
static bool isCircularGreater(unsigned long x1, unsigned long x2) {
	unsigned long d1 = x1 - x2;
	unsigned long d2 = x2 - x1;
	return d1 < d2;
}

bool Ribosome::step() {
	bool hasFirst = crtPosition_ < bug_->genome_.first.size();
	bool hasSecond = crtPosition_ < bug_->genome_.second.size();
	if (!hasFirst && !hasSecond) {
		// decoding sequence finished
		// apply the general attribute genes:
		for (auto &g : generalAttribGenes)
			decodeGeneralAttrib(g);
		return false;
	}
	Gene* g = nullptr;
	// choose the dominant (or the only) gene out of the current pair:
	if (hasFirst && (!hasSecond || isCircularGreater(
			bug_->genome_.first[crtPosition_].RID,
			bug_->genome_.second[crtPosition_].RID)
		))
		g = &bug_->genome_.first[crtPosition_];
	else
		g = &bug_->genome_.second[crtPosition_];

	// now decode the gene
	switch (g->type) {
	case GENE_TYPE_LOCATION:
		activeSet_.clear();
		root_->matchLocation(g->data.gene_location.location, constants::MAX_GROWTH_DEPTH, &activeSet_);
		break;
	case GENE_TYPE_DEVELOPMENT:
		decodeDevelopCommand(g->data.gene_command);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		decodePartAttrib(g->data.gene_local_attribute);
		break;
	case GENE_TYPE_GENERAL_ATTRIB:
		// postpone these genes and apply them at the end, because they must apply to the whole body
		generalAttribGenes.push_back(g->data.gene_general_attribute);
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
	crtPosition_++;
	return true;
}

bool Ribosome::partMustGenerateJoint(int part_type) {
	switch (part_type) {
	case GENE_PART_BONE:
	case GENE_PART_GRIPPER:
		return true;
	default:
		return false;
	}
}

void Ribosome::decodeDevelopCommand(GeneCommand const& g) {
	if (g.command == GENE_DEV_GROW) {
		decodeDevelopGrowth(g);
	} else if (g.command == GENE_DEV_SPLIT) {
		decodeDevelopSplit(g);
	}
}

void Ribosome::decodeDevelopGrowth(GeneCommand const& g) {
	// now grow a new part on each adequate element in nodes list
	for (auto n : activeSet_) {
		// grow only works on bones and torso
		if (n->getType() != BODY_PART_BONE && n->getType() != BODY_PART_TORSO)
			continue;
		if (n->getChildrenCount() == BodyPart::MAX_CHILDREN)
			continue;

		float angle = g.angle;

		// The child's attachment point relative to the parent's center is computed from the angle specified in the gene,
		// by casting a ray from the parent's origin in the specified angle (which is relative to the parent's orientation)
		// until it touches an edge of the parent. That point is used as attachment of the new part.
		// glm::vec2 offset = n->bodyPart->getChildAttachmentPoint(angle);

		if (partMustGenerateJoint(g.part_type)) {
			// we cannot grow this part directly onto its parent, they must be connected by a joint
			Joint* linkJoint = new Joint(n);
			linkJoint->getAttribute(GENE_ATTRIB_ATTACHMENT_ANGLE)->reset(angle);

			// now generate the two muscles around the joint
			// 1. Left
			if (n->getChildrenCount() < BodyPart::MAX_CHILDREN) {
				float mLeftAngle = angle + MUSCLE_OFFSET_ANGLE;
				Muscle* mLeft = new Muscle(n, linkJoint, +1);
				mLeft->getAttribute(GENE_ATTRIB_ATTACHMENT_ANGLE)->reset(mLeftAngle);
				mLeft->getAttribute(GENE_ATTRIB_LOCAL_ROTATION)->reset(-MUSCLE_OFFSET_ANGLE);
				int motorLineId = bug_->motors_.size();
				bug_->motors_.push_back(mLeft);
				mLeft->addMotorLine(motorLineId);
			}
			// 2. Right
			if (n->getChildrenCount() < BodyPart::MAX_CHILDREN) {
				float mRightAngle = angle - MUSCLE_OFFSET_ANGLE;
				Muscle* mRight = new Muscle(n, linkJoint, -1);
				mRight->getAttribute(GENE_ATTRIB_ATTACHMENT_ANGLE)->reset(mRightAngle);
				mRight->getAttribute(GENE_ATTRIB_LOCAL_ROTATION)->reset(+MUSCLE_OFFSET_ANGLE);
				int motorLineId = bug_->motors_.size();
				bug_->motors_.push_back(mRight);
				mRight->addMotorLine(motorLineId);
			}

			// set n to point to the joint's node, since that's where the actual part will be attached:
			n = linkJoint;
			// recompute coordinates in joint's space:
			angle = 0;
			// offset = n->bodyPart->getChildAttachmentPoint(0);
		}

		BodyPart* bp = nullptr;
		switch (g.part_type) {
		case GENE_PART_BONE:
			bp = new Bone(n);
			break;
		case GENE_PART_GRIPPER: {
			Gripper* g = new Gripper(n);
			int motorLineId = bug_->motors_.size();
			bug_->motors_.push_back(g);
			g->addMotorLine(motorLineId);
			bp = g;
		}
			break;
		case GENE_PART_SENSOR:
			// bp = new sensortype?(n->bodyPart, PhysicsProperties(offset, angle));
			break;
		default:
			break;
		}
		if (!bp)
			continue;
		bp->getAttribute(GENE_ATTRIB_ATTACHMENT_ANGLE)->reset(angle);
	}
}
void Ribosome::decodeDevelopSplit(GeneCommand const& g) {
	// split may work on bones, joints and grippers only
	for (auto n : activeSet_) {
		if (   n->getType() != BODY_PART_BONE
			&& n->getType() != BODY_PART_JOINT
			&& n->getType() != BODY_PART_GRIPPER
			)
			continue;
	}
	// split on bone or gripper actually splits its parent joint
	// split on joint also duplicates the muscles around the joint
	// angle of gene represents the angle to separate the newly split parts
}

void Ribosome::decodePartAttrib(GeneLocalAttribute const& g) {
	for (auto n : activeSet_) {
		CummulativeValue* pAttrib = n->getAttribute((gene_part_attribute_type)g.attribute);
		if (pAttrib)
			pAttrib->changeAbs(g.value);
	}
}

void Ribosome::decodeGeneralAttrib(GeneGeneralAttribute const& g) {
	root_->applyRecursive([&g] (BodyPart* n) {
		CummulativeValue* pAttrib = n->getAttribute((gene_part_attribute_type)g.attribute);
		if (pAttrib)
			pAttrib->changeRel(g.value);
	});
}

void Ribosome::decodeSynapse(GeneSynapse const& g) {

}

void Ribosome::decodeTransferFn(GeneTransferFunction const& g) {

}

void Ribosome::decodeMuscleCommand(GeneMuscleCommand const& g) {

}
