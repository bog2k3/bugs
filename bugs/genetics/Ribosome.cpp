#include "Ribosome.h"

#include "../neuralnet/functions.h"
#include "../utils/rand.h"
#include "../math/math2D.h"
#include "../utils/log.h"
#include "../neuralnet/Network.h"
#include "../neuralnet/Neuron.h"
#include "../neuralnet/OutputSocket.h"
#include "../neuralnet/functions.h"
#include "Gene.h"
#include "GeneDefinitions.h"
#include "CummulativeValue.h"
#include "../entities/Bug.h"
#include "../body-parts/BodyPart.h"
#include "../body-parts/Torso.h"
#include "../body-parts/Bone.h"
#include "../body-parts/Gripper.h"
#include "../body-parts/Joint.h"
#include "../body-parts/ZygoteShell.h"
#include "../body-parts/Muscle.h"
#include "../body-parts/Mouth.h"
#include "../utils/log.h"
#include "../neuralnet/InputSocket.h"

#include "Genome.h"
using namespace std;

static constexpr float MUSCLE_OFFSET_ANGLE = PI * 0.25f;

Ribosome::Ribosome(Bug* bug)
	: bug_{bug}
	, crtPosition_{0}
{
	// create default body parts:
	// 1. mouth
	bug->body_->setMouth(new Mouth(bug_->body_));		// this will be the location 0x1

	// 2. Egg-layer
	// ...
}

Ribosome::~Ribosome() {
	cleanUp();
}

void Ribosome::cleanUp() {
	generalAttribGenes.clear();
	neuralGenes.clear();
	activeSet_.clear();
	mapNeurons_.clear();
	mapSynapses.clear();
	mapFeedbackSynapses.clear();
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

void Ribosome::initializeNeuralNetwork() {
	// create and initialize the neural network:
	bug_->neuralNet_ = new NeuralNet();
	bug_->neuralNet_->inputs.reserve(bug_->sensors_.size());
	for (unsigned i=0; i<bug_->sensors_.size(); i++)
		bug_->neuralNet_->inputs.push_back(bug_->sensors_[i]->getOutSocket());
	bug_->neuralNet_->outputs.reserve(bug_->motors_.size());
	for (unsigned i=0; i<bug_->motors_.size(); i++)
		bug_->neuralNet_->outputs.push_back(bug_->motors_[i]->getInputSocket());	// create network outputs
	// create neurons:
	int commandNeuronsStart = mapNeurons_.size();
	int totalNeurons = commandNeuronsStart + bug_->motors_.size();
	bug_->neuralNet_->neurons.reserve(totalNeurons);
	for (int i=0; i<commandNeuronsStart; i++) {
		bug_->neuralNet_->neurons.push_back(new Neuron());
	}
	// create and initialize the output neurons:
	for (int i=commandNeuronsStart; i<totalNeurons; i++) {
		bug_->neuralNet_->neurons.push_back(new Neuron());
		bug_->neuralNet_->neurons[i]->neuralConstant = 0.f;
		bug_->neuralNet_->neurons[i]->transfFunc = mapTransferFunctions[FN_ONE];
		bug_->neuralNet_->neurons[i]->output.addTarget(bug_->neuralNet_->outputs[i-commandNeuronsStart].get());
	}
}

void Ribosome::decodeDeferredGenes() {
	int commandNeuronsStart = mapNeurons_.size();
	// create all synapses
	for (auto s : mapSynapses) {
		int32_t from = s.first >> 32;
		int32_t to = (s.first) & 0xFFFFFFFF;
		createSynapse(from, to, commandNeuronsStart, s.second);
	}
	for (auto s : mapFeedbackSynapses) {
		int32_t from = s.first >> 32;
		int32_t to = (s.first) & 0xFFFFFFFF;
		createFeedbackSynapse(from, to, commandNeuronsStart, s.second);
	}
	// now decode the deferred neural genes (neuron properties):
	for (auto &g : neuralGenes)
		decodeGene(*g, false);
	// apply all neuron properties
	for (auto n : mapNeurons_) {
		if (n.second.transfer.hasValue()) {
			bug_->neuralNet_->neurons[n.second.index]->transfFunc = mapTransferFunctions[(transferFuncNames)n.second.transfer.get()];
			if (bug_->neuralNet_->neurons[n.second.index]->transfFunc == nullptr)
				bug_->neuralNet_->neurons[n.second.index]->transfFunc = mapTransferFunctions[FN_ONE];
		} if (n.second.constant.hasValue())
			bug_->neuralNet_->neurons[n.second.index]->neuralConstant = n.second.constant;
	}

	// apply the general attribute genes:
	for (auto &g : generalAttribGenes)
		decodeGeneralAttrib(*g);
}

bool Ribosome::step() {
	bool hasFirst = crtPosition_ < bug_->genome_.first.size();
	bool hasSecond = crtPosition_ < bug_->genome_.second.size();
	if (!hasFirst && !hasSecond) {
		// reached the end of the genome.
		// finish up with neural network...
		initializeNeuralNetwork();
		//...and all deferred genes:
		decodeDeferredGenes();

		// clean up:
		cleanUp();

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

	/*
	 * 1. Must automatically generate muscles for joints;
	 * 2. Must automatically generate life time sensor
	 * 3. Auto-generate output neurons to command actuators (muscles, grippers, etc)
	 * 4. Auto-generate Mouth
	 * 5. Auto-generate body-part-sensors in joints & grippers and other parts that may have useful info
	 */

	// now decode the gene
	decodeGene(*g, true);

	// move to next position
	crtPosition_++;
	return true;
}

void Ribosome::checkAndAddNeuronMapping(int virtualIndex) {
	if (virtualIndex >= 0) {	// does it refer to an actual neuron? (skip <0 input & output sockets)
		if (!hasNeuron(virtualIndex)) {
			int realIndex = mapNeurons_.size();
			mapNeurons_[virtualIndex] = NeuronInfo(realIndex);
		}
	}
}

void Ribosome::updateNeuronTransfer(int virtualIndex, float transfer) {
	if (hasNeuron(virtualIndex)) {
		mapNeurons_[virtualIndex].transfer.changeAbs(transfer);
	}
}

void Ribosome::updateNeuronConstant(int virtualIndex, float constant) {
	if (hasNeuron(virtualIndex)) {
		mapNeurons_[virtualIndex].constant.changeAbs(constant);
	}
}

void Ribosome::decodeGene(Gene const& g, bool deferNeural) {
	switch (g.type) {
	case GENE_TYPE_LOCATION:
		activeSet_.clear();
		bug_->body_->matchLocation(g.data.gene_location.location, constants::MAX_GROWTH_DEPTH, &activeSet_);
		break;
	case GENE_TYPE_DEVELOPMENT:
		decodeDevelopCommand(g.data.gene_command);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		decodePartAttrib(g.data.gene_local_attribute);
		break;
	case GENE_TYPE_GENERAL_ATTRIB:
		// postpone these genes and apply them at the end, because they must apply to the whole body
		generalAttribGenes.push_back(&g.data.gene_general_attribute);
		break;
	case GENE_TYPE_BODY_ATTRIBUTE:
		bug_->mapBodyAttributes_[g.data.gene_body_attribute.attribute]->changeAbs(g.data.gene_body_attribute.value);
		break;
	case GENE_TYPE_SYNAPSE:
		decodeSynapse(g.data.gene_synapse);
		break;
	case GENE_TYPE_FEEDBACK_SYNAPSE:
		decodeFeedbackSynapse(g.data.gene_feedback_synapse);
		break;
	case GENE_TYPE_TRANSFER:
		if (deferNeural)
			neuralGenes.push_back(&g);
		else
			decodeTransferFn(g.data.gene_transfer_function);
		break;
	case GENE_TYPE_NEURAL_CONST:
		if (deferNeural)
			neuralGenes.push_back(&g);
		else
			decodeNeuralConst(g.data.gene_neural_constant);
		break;
	default:
		ERROR("Unhandled gene type : " << g.type);
	}
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
	bug_->body_->applyRecursive([&g] (BodyPart* n) {
		CummulativeValue* pAttrib = n->getAttribute((gene_part_attribute_type)g.attribute);
		if (pAttrib)
			pAttrib->changeRel(g.value);
	});
}

void Ribosome::decodeSynapse(GeneSynapse const& g) {
	// the number of neurons is derived from the synapse values
	checkAndAddNeuronMapping(g.from);
	checkAndAddNeuronMapping(g.to);
	int64_t key = synKey(g.from, g.to);
	mapSynapses[key].changeAbs(g.weight);
}

void Ribosome::decodeFeedbackSynapse(GeneFeedbackSynapse const& g) {
	// the number of neurons is derived from the synapse values
	checkAndAddNeuronMapping(g.to);
	int64_t key = synKey(g.from, g.to);
	mapFeedbackSynapses[key].changeAbs(g.weight);
}

void Ribosome::decodeTransferFn(GeneTransferFunction const& g) {
	updateNeuronTransfer(g.targetNeuron, g.functionID);
}

void Ribosome::decodeNeuralConst(GeneNeuralConstant const& g) {
	updateNeuronConstant(g.targetNeuron, g.value);
}

void Ribosome::createSynapse(int from, int to, int commandNeuronsOfs, float weight) {
	OutputSocket* pFrom = nullptr;
	if (from < 0) { // this apparently comes from an input socket
		if (-from <= (int)bug_->neuralNet_->inputs.size())
			pFrom = bug_->neuralNet_->inputs[-from-1].get();	// map -1..-n to 0..n-1
		else
			return;	// invalid index
	} else {
		assert(hasNeuron(from));	// should be there, since synapses dictate neurons
		pFrom = &bug_->neuralNet_->neurons[mapNeurons_[from].index]->output;
	}
	Neuron* pTo;
	if (to < 0) { // apparently a motor command output socket
		if (-to <= (int)bug_->neuralNet_->outputs.size())
			pTo = bug_->neuralNet_->neurons[commandNeuronsOfs - to - 1];
		else
			return; // invalid index
	} else {
		assert(hasNeuron(to));
		pTo = bug_->neuralNet_->neurons[mapNeurons_[to].index];
	}

	InputSocket* i = new InputSocket(pTo, weight);
	pTo->inputs.push_back(std::unique_ptr<InputSocket>(i));
	pFrom->addTarget(i);
}

void Ribosome::createFeedbackSynapse(int from, int to, int commandNeuronsOfs, float weight) {
	OutputSocket* pFrom = nullptr;
	// from must be between [0, numOutputs-1]
	if (from < 0 || from >= (int)bug_->neuralNet_->outputs.size())
		return;	// invalid index
	pFrom = &bug_->neuralNet_->neurons[commandNeuronsOfs + from]->output;

	Neuron* pTo;
	if (to < 0) { // apparently a motor command output socket
		if (-to <= (int)bug_->neuralNet_->outputs.size())
			pTo = bug_->neuralNet_->neurons[commandNeuronsOfs - to - 1];
		else
			return; // invalid index
	} else {
		assert(hasNeuron(to));
		pTo = bug_->neuralNet_->neurons[mapNeurons_[to].index];
	}

	InputSocket* i = new InputSocket(pTo, weight);
	pTo->inputs.push_back(std::unique_ptr<InputSocket>(i));
	pFrom->addTarget(i);
}
