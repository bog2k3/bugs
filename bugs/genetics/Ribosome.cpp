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
#include "Genome.h"
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
#include "../body-parts/EggLayer.h"
#include "../utils/log.h"
#include "../neuralnet/InputSocket.h"

#include <utility>

Ribosome::Ribosome(Bug* bug)
	: bug_{bug}
{
	// there are no more default body parts; they either get created by the genes, or the embryo
	// is discarded at the end of development if it lacks critical parts such as mouth or egg-layer

	// start decoding with root body part at offset 0 in the genome:
	activeSet_.push_back(std::make_pair(bug_->body_, 0));

	// the number of default sensors (like lifeTime sensor) added by the bug before genetic decoding
	nDefaultSensors = bug_->sensors_.size();
}

Ribosome::~Ribosome() {
	cleanUp();
}

void Ribosome::cleanUp() {
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
static bool isCircularGreater(decltype(Gene::RID) x1, decltype(Gene::RID) x2) {
	decltype(Gene::RID) d1 = x1 - x2;
	decltype(Gene::RID) d2 = x2 - x1;
	return d1 < d2;
}

void Ribosome::initializeNeuralNetwork() {
	// create and initialize the neural network:
	bug_->neuralNet_ = new NeuralNet();
	bug_->neuralNet_->inputs.reserve(bug_->sensors_.size());
	for (unsigned i=0; i<bug_->sensors_.size(); i++) {
		// sort sensors by genetic age
		if ((int)i>=nDefaultSensors) // skip the default sensors which must always be at the beginning
			for (unsigned j=i+1; j<bug_->sensors_.size(); j++)
				if (bug_->sensors_[j]->geneticAge > bug_->sensors_[i]->geneticAge)
					xchg(bug_->sensors_[i], bug_->sensors_[j]);
		bug_->neuralNet_->inputs.push_back(bug_->sensors_[i]->getOutSocket());
	}
	bug_->neuralNet_->outputs.reserve(bug_->motors_.size());
	std::map<int, int> mapMotorIds, mapReverseMotorIds;
	for (unsigned i=0; i<bug_->motors_.size(); i++) {
		// sort motors by genetic age (oldest first)
		for (unsigned j=i+1; j<bug_->motors_.size(); j++)
			if (bug_->motors_[j].geneticAge > bug_->motors_[i].geneticAge) {
				xchg(bug_->motors_[i], bug_->motors_[j]);
				// update mapping of indices:
				auto it1 = mapReverseMotorIds.find(i);
				int indexWhereKeyIs_i = i;
				if (it1 != mapReverseMotorIds.end())
					indexWhereKeyIs_i = it1->second;
				auto it2 = mapReverseMotorIds.find(j);
				int indexWhereKeyIs_j = j;
				if (it2 != mapReverseMotorIds.end())
					indexWhereKeyIs_j = it2->second;
				mapMotorIds[indexWhereKeyIs_i] = j;
				mapMotorIds[indexWhereKeyIs_j] = i;
				mapReverseMotorIds[j] = indexWhereKeyIs_i;
				mapReverseMotorIds[i] = indexWhereKeyIs_j;
			}
		// now add all input sockets from all motors:
		bug_->neuralNet_->outputs.push_back(bug_->motors_[i].inputSocket);	// create network outputs
	}
	bug_->body_->updateMotorMappings(mapMotorIds);
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
		bug_->neuralNet_->neurons[i]->transfFunc = mapTransferFunctions[transferFuncNames::FN_ONE];
		bug_->neuralNet_->neurons[i]->output.addTarget(bug_->neuralNet_->outputs[i-commandNeuronsStart].get());
		// connect back the socket to the output neuron (in order to be able to locate the neuron later):
		bug_->neuralNet_->outputs[i-commandNeuronsStart]->pParentNeuron = bug_->neuralNet_->neurons[i];
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
		decodeGene(*g, nullptr, nullptr, false);
	// apply all neuron properties
	for (auto n : mapNeurons_) {
		if (n.second.transfer.hasValue()) {
			int funcIndex = clamp((int)n.second.transfer.get(),
					(int)transferFuncNames::FN_ONE,
					(int)transferFuncNames::FN_MAXCOUNT-1);
			bug_->neuralNet_->neurons[n.second.index]->transfFunc = mapTransferFunctions[(transferFuncNames)funcIndex];
		}
		if (n.second.constant.hasValue())
			bug_->neuralNet_->neurons[n.second.index]->neuralConstant = n.second.constant;
	}
}

bool Ribosome::step() {
	if (activeSet_.empty()) {
		// finished decoding all body parts.

		// check if critical body parts exist (at least a mouth and egg-layer)
		bool hasMouth = false, hasEggLayer = false;
		bug_->body_->applyRecursive([&hasMouth, &hasEggLayer, this] (BodyPart* p) {
			if (p->getType() == BODY_PART_MOUTH)
				hasMouth = true;
			if (p->getType() == BODY_PART_EGGLAYER) {
				hasEggLayer = true;
				((EggLayer*)p)->setTargetEggMass(bug_->eggMass_);
			}
			return hasMouth && hasEggLayer;
		});
		if (!hasMouth || !hasEggLayer) {
			// here mark the embryo as dead and return
			bug_->isAlive_ = false;
			cleanUp();
			return false;
		}

		// now decode the neural network:
		initializeNeuralNetwork();
		decodeDeferredGenes();

		// clean up:
		cleanUp();

		return false;
	}
	int nCrtBranches = activeSet_.size();
	for (int i=0; i<nCrtBranches; i++) {
		BodyPart* p = activeSet_[i].first;
		unsigned offset = activeSet_[i].second.crtGenomePos++;
		bool hasFirst = offset < bug_->genome_.first.genes.size();
		bool hasSecond = offset < bug_->genome_.second.genes.size();
		bool reachedTheEnd = !hasFirst && !hasSecond;
		if (hasFirst)
			hasFirst = bug_->genome_.first.genes[offset].type != GENE_TYPE_NO_OP;
		if (hasSecond)
			hasSecond = bug_->genome_.second.genes[offset].type != GENE_TYPE_NO_OP;
		if (!hasFirst && !hasSecond && !reachedTheEnd)
			continue;
		Gene* g = nullptr;
		if (!reachedTheEnd) {
			// choose the dominant (or the only) gene out of the current pair:
			if (hasFirst && (!hasSecond || isCircularGreater(
					bug_->genome_.first.genes[offset].RID,
					bug_->genome_.second.genes[offset].RID)
				))
				g = &bug_->genome_.first.genes[offset];
			else
				g = &bug_->genome_.second.genes[offset];
		}
		if (reachedTheEnd || g->type == GENE_TYPE_STOP) {
			// so much for this development path;
			// grow body parts from all segments now
			for (int k=0; k<MAX_CHILDREN; k++)
				growBodyPart(p, k, activeSet_[i].second.hyperPositions[k],
						activeSet_[i].second.startGenomePos + activeSet_[i].second.offsets[k]);
			// and remove this branch:
			activeSet_.erase(activeSet_.begin()+i);
			i--, nCrtBranches--;
			continue;
		}
		if (g->type == GENE_TYPE_SKIP) {
			int depth = p->getDepth();
			if (depth <= g->data.gene_skip.maxDepth && depth >= g->data.gene_skip.minDepth) {
				activeSet_[i].second.crtGenomePos += g->data.gene_skip.count;
			}
			continue;
		}
		// now decode the gene
		decodeGene(*g, p, &activeSet_[i].second, true);
	}
	return true;
}

void Ribosome::growBodyPart(BodyPart* parent, int attachmentSegment, glm::vec4 hyperPosition, int genomeOffset) {
	// grow only works on bones and torso
	if (parent->getType() != BODY_PART_BONE && parent->getType() != BODY_PART_TORSO)
		return;
	// determine the body part type to grow from the hyperPosition
	PART_TYPE partTypes[2][2][2][2] = {
		/* W- */ {
			/* Z- */ {
				/* Y- */ {
					/* X- */ BODY_PART_BONE, /* X+ */ BODY_PART_INVALID
				},
				/* Y+ */ {
					/* X- */ BODY_PART_GRIPPER, /* X+ */ BODY_PART_MOUTH
				},
			},
			/* Z+ */ {
				/* Y- */ {
					/* X- */ BODY_PART_INVALID, /* X+ */ BODY_PART_INVALID
				},
				/* Y+ */ {
					/* X- */ BODY_PART_MUSCLE, /* X+ */ BODY_PART_EGGLAYER
				},
			},
		},
		/* W+ */ {
			/* Z- */ {
				/* Y- */ {
					/* X- */ BODY_PART_INVALID, /* X+ */ BODY_PART_INVALID
				},
				/* Y+ */ {
					/* X- */ BODY_PART_SENSOR_PROXIMITY, /* X+ */ BODY_PART_SENSOR_DIRECTION
				},
			},
			/* Z+ */ {
				/* Y- */ {
					/* X- */ BODY_PART_INVALID, /* X+ */ BODY_PART_INVALID
				},
				/* Y+ */ {
					/* X- */ BODY_PART_SENSOR_COMPASS, /* X+ */ BODY_PART_SENSOR_SIGHT
				},
			},
		}
	};
	// if any one axis is zero, we cannot determine the part type and none is grown
	if (hyperPosition.x * hyperPosition.y * hyperPosition.z * hyperPosition.w == 0)
		return;
	PART_TYPE newPartType = partTypes[hyperPosition.w > 0][hyperPosition.z > 0][hyperPosition.y > 0][hyperPosition.x > 0];
	if (newPartType == BODY_PART_INVALID)
		return;

	// TODO Auto-generate body-part-sensors in joints & grippers and other parts that may have useful info

	/*int age = g.age;
	if (mapGeneToIterations_[&g]++ > 0) {
		// this is not the first time we're reading this gene
		if (!g.rereadAgeOffset)
			g.rereadAgeOffset = -g.age;
		// rereadAgeOffset must be carried over into the next generation
		age += g.rereadAgeOffset;
	}*/

	float angle = attachmentSegment * 2*PI / MAX_CHILDREN;

	// The child's attachment point relative to the parent's center is computed from the angle of the current segment,
	// by casting a ray from the parent's origin in the specified angle (which is relative to the parent's orientation)
	// until it touches an edge of the parent. That point is used as attachment of the new part.

	if (partMustGenerateJoint(newPartType)) {
		// we cannot grow this part directly onto its parent, they must be connected by a joint
		Joint* linkJoint = new Joint();
		/*// now generate the two muscles around the joint
		// 1. Right
		Muscle* mRight = nullptr;
		if (part->getChildrenCount() < MAX_CHILDREN) {
			float mRightAngle = angle - 0.01f;
			mRight = new Muscle(linkJoint, -1);
			float origAngle = mRightAngle;
			mRightAngle = part->add(mRight, mRightAngle);
			mRightAngle = limitAngle(mRightAngle-origAngle, PI) + origAngle;
			mRight->getAttribute(GENE_ATTRIB_LOCAL_ROTATION)->reset(angle - mRightAngle);
			int motorLineId = bug_->motors_.size();
			bug_->motors_.push_back(Motor(mRight->getInputSocket(), age));
			mRight->addMotorLine(motorLineId);
			activeSet_.push_back(std::make_pair(mRight, crtPosition + g.genomeOffsetMuscle2));
		}
		// 2. Left
		Muscle* mLeft = nullptr;
		if (part->getChildrenCount() < MAX_CHILDREN) {
			float mLeftAngle = angle + 0.01f;
			mLeft = new Muscle(linkJoint, +1);
			float origAngle = mLeftAngle;
			mLeftAngle = part->add(mLeft, mLeftAngle);
			mLeftAngle = limitAngle(mLeftAngle-origAngle, PI) + origAngle;
			mLeft->getAttribute(GENE_ATTRIB_LOCAL_ROTATION)->reset(angle - mLeftAngle);
			int motorLineId = bug_->motors_.size();
			bug_->motors_.push_back(Motor(mLeft->getInputSocket(), age));
			mLeft->addMotorLine(motorLineId);
			activeSet_.push_back(std::make_pair(mLeft, crtPosition + g.genomeOffsetMuscle1));
		}*/
		parent->add(linkJoint, angle);

		// set part to point to the joint's node, since that's where the actual part will be attached:
		parent = linkJoint;
		// recompute coordinates in joint's space:
		angle = 0;
	}

	BodyPart* bp = nullptr;
	switch (newPartType) {
	case BODY_PART_BONE:
		bp = new Bone();
		break;
	case BODY_PART_GRIPPER: {
		Gripper* gr = new Gripper();
		int motorLineId = bug_->motors_.size();
		bug_->motors_.push_back(Motor(gr->getInputSocket(), age));
		gr->addMotorLine(motorLineId);
		bp = gr;
		break;
	}
	case BODY_PART_MUSCLE: {
		// todo
		break;
	}
	case BODY_PART_MOUTH: {
		Mouth* m = new Mouth();
		bp = m;
		break;
	}
	case BODY_PART_SENSOR_COMPASS:
		// bp = new sensortype?(part->bodyPart, PhysicsProperties(offset, angle));
		break;
	case BODY_PART_SENSOR_DIRECTION:
		// bp = new sensortype?(part->bodyPart, PhysicsProperties(offset, angle));
		break;
	case BODY_PART_SENSOR_PROXIMITY:
		// bp = new sensortype?(part->bodyPart, PhysicsProperties(offset, angle));
		break;
	case BODY_PART_SENSOR_SIGHT:
		// bp = new sensortype?(part->bodyPart, PhysicsProperties(offset, angle));
		break;
	case BODY_PART_EGGLAYER: {
		EggLayer* e = new EggLayer();
		for (auto &is : e->getInputSockets())
			bug_->motors_.push_back(Motor(is, age));
		bug_->eggLayers_.push_back(e);
		bp = e;
		break;
	}
	default:
		ERROR("unhandled gene part type: "<<newPartType);
		break;
	}
	if (!bp)
		return;

	parent->add(bp, angle);

	// start a new development path from the new part:
	activeSet_.push_back(std::make_pair(bp, genomeOffset));
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

void Ribosome::decodeGene(Gene &g, BodyPart* part, GrowthData *growthData, bool deferNeural) {
#warning: "only from depth 0 (torso) must the neural genes be taken into account (?)"
	switch (g.type) {
	case GENE_TYPE_NO_OP:
		break;
	case GENE_TYPE_PROTEIN:
		decodeProtein(g.data.gene_protein, part, growthData);
		break;
	case GENE_TYPE_OFFSET:
		decodeOffset(g.data.gene_offset, part, growthData);
		break;
	case GENE_TYPE_PART_ATTRIBUTE:
		decodePartAttrib(g.data.gene_attribute, part);
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
	case BODY_PART_BONE:
	case BODY_PART_GRIPPER:
		return true;
	default:
		return false;
	}
}

void Ribosome::decodeProtein(GeneProtein &g, BodyPart* part, GrowthData *growthData) {
	int crtDepth = part->getDepth();
	if (crtDepth < g.minDepth || crtDepth > g.maxDepth)
		return;
	glm::vec4 &pos = growthData->hyperPositions[g.targetSegment];
	switch (g.protein) {
	case GENE_PROT_A:
		pos.x--;
		break;
	case GENE_PROT_B:
		pos.x++;
		break;
	case GENE_PROT_C:
		pos.y--;
		break;
	case GENE_PROT_D:
		pos.y++;
		break;
	case GENE_PROT_E:
		pos.z--;
		break;
	case GENE_PROT_F:
		pos.z++;
		break;
	case GENE_PROT_G:
		pos.w--;
		break;
	case GENE_PROT_H:
		pos.w++;
		break;
	}
}

void Ribosome::decodeOffset(GeneOffset &g, BodyPart *part, GrowthData *growthData) {
	int crtDepth = part->getDepth();
	if (crtDepth < g.minDepth || crtDepth > g.maxDepth)
		return;
	growthData->offsets[g.targetSegment].changeAbs(g.offset);
}

void Ribosome::decodePartAttrib(GeneAttribute const& g, BodyPart* part) {
	int depth = part->getDepth();
	if (depth >= g.minDepth && depth <= g.maxDepth)
	{
		CummulativeValue* pAttrib = part->getAttribute(g.attribute);
		if (pAttrib)
			pAttrib->changeAbs(g.value);
	}
}

void Ribosome::decodeSynapse(GeneSynapse const& g) {
	// the number of neurons is derived from the synapse values
	checkAndAddNeuronMapping(g.from);
	checkAndAddNeuronMapping(g.to);
	int64_t key = synKey(g.from, g.to);
	assert(!std::isnan(g.weight.value));
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
	assert(!std::isnan(g.value.value));
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
		assertDbg(hasNeuron(from));	// should be there, since synapses dictate neurons
		pFrom = &bug_->neuralNet_->neurons[mapNeurons_[from].index]->output;
	}
	Neuron* pTo;
	if (to < 0) { // apparently a motor command output socket
		if (-to <= (int)bug_->neuralNet_->outputs.size())
			pTo = bug_->neuralNet_->neurons[commandNeuronsOfs - to - 1];
		else
			return; // invalid index
	} else {
		assertDbg(hasNeuron(to));
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
		assertDbg(hasNeuron(to));
		pTo = bug_->neuralNet_->neurons[mapNeurons_[to].index];
	}

	InputSocket* i = new InputSocket(pTo, weight);
	pTo->inputs.push_back(std::unique_ptr<InputSocket>(i));
	pFrom->addTarget(i);
}
