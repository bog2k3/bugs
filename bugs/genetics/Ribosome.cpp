#include "Ribosome.h"

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
#include "../body-parts/sensors/Nose.h"
#include "../neuralnet/functions.h"
#include "../neuralnet/Network.h"
#include "../neuralnet/Neuron.h"
#include "../neuralnet/OutputSocket.h"
#include "../neuralnet/functions.h"
#include "../neuralnet/InputSocket.h"
#include "Gene.h"
#include "Genome.h"
#include "GeneDefinitions.h"
#include "CummulativeValue.h"
#include "../utils/log.h"
#include "../math/math3D.h"
#include "../utils/rand.h"
#include "../utils/log.h"

#include <utility>
#include <algorithm>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

Ribosome::Ribosome(Bug* bug)
	: bug_{bug}
{
	// there are no default body parts; they either get created by the genes, or the embryo
	// is discarded at the end of development if it lacks critical parts such as mouth or egg-layer

	// start decoding with root body part at offset 0 in the genome:
	activeSet_.push_back(std::make_pair(bug_->body_, 0));
}

Ribosome::~Ribosome() {
	cleanUp();
}

void Ribosome::cleanUp() {
	neuralGenes_.clear();
	activeSet_.clear();
	mapNeurons_.clear();
	mapSynapses_.clear();
	outputNeurons_.clear();
	inputNeurons_.clear();
	motors_.clear();
	sensors_.clear();
	mapInputNerves_.clear();
}

// compares two unsigned longs as if they were expressed as coordinates in a circular scale
// (where the largest number comes right before 0 and is considered smaller than 0)
// X1 is greater than X2 on this scale if X1 is to the left of X2 (the coordinates grow in
// geometrical direction - counter-clockwise). That means the path from X1 back to X2 is shorter
// than the path from X1 forward to X2.
// This model guarantees that any number has half other numbers greater than it and half smaller than it,
// which is a needed condition for gene dominance, so that no number is privileged in an absolute manner,
// only relative to other genes.
//static bool isCircularGreater(decltype(Gene::RID) x1, decltype(Gene::RID) x2) {
//	decltype(Gene::RID) d1 = x1 - x2;
//	decltype(Gene::RID) d2 = x2 - x1;
//	return d1 < d2;
//}

void Ribosome::initializeNeuralNetwork() {
	// create and initialize the neural network:
	bug_->neuralNet_ = new NeuralNet();
	bug_->neuralNet_->neurons.reserve(mapNeurons_.size());
	for (uint i=0; i<mapNeurons_.size(); i++) {
		bug_->neuralNet_->neurons.push_back(new Neuron());
	}
#ifdef DEBUG
	for (auto const& it : mapNeurons_) {
		mapNeuronVirtIndex_[bug_->neuralNet_->neurons[it.second.index]] = it.first;
		if (false) {
			LOGLN("Neuron MAPPING: " << it.first << "(v) -> " << it.second.index << "(r)" << "\t" << bug_->neuralNet_->neurons[it.second.index]);
		}
	}
#endif
}

void Ribosome::decodeDeferredGenes() {
	// create all synapses
	for (auto s : mapSynapses_) {
		int32_t from = s.first >> 32;
		int32_t to = (s.first) & 0xFFFFFFFF;
		createSynapse(from, to, s.second);
	}
	// now decode the deferred neural genes (neuron properties):
	for (auto &g : neuralGenes_)
		decodeGene(*g, nullptr, nullptr, false);
	// apply all neuron properties
	for (auto &n : mapNeurons_) {
		if (n.second.transfer.hasValue()) {
			int funcIndex = clamp((int)n.second.transfer.get(),
					(int)transferFuncNames::FN_ONE,
					(int)transferFuncNames::FN_MAXCOUNT-1);
			bug_->neuralNet_->neurons[n.second.index]->setTranferFunction((transferFuncNames)funcIndex);
		}
		if (n.second.bias.hasValue())
			bug_->neuralNet_->neurons[n.second.index]->inputBias = n.second.bias;
		if (n.second.param.hasValue())
			bug_->neuralNet_->neurons[n.second.index]->neuralParam = n.second.param;
	}
}

template <typename T>
void Ribosome::sortNervesByVMSCoord(std::vector<InputOutputNerve<T>> &nerves) {
	std::sort(nerves.begin(), nerves.end(), [] (InputOutputNerve<T> const& left, InputOutputNerve<T> const& right) -> bool {
		return left.second < right.second;
	});
}

bool Ribosome::step() {
	LOGPREFIX("Ribosome");
	if (activeSet_.empty()) {
		// finished decoding all body parts.

		// check if critical body parts exist (at least a mouth and egg-layer)
		bool hasMouth = false, hasEggLayer = false;
		bug_->body_->applyRecursive([&hasMouth, &hasEggLayer, this] (BodyPart* p) {
			if (p->getType() == BodyPartType::MOUTH)
				hasMouth = true;
			if (p->getType() == BodyPartType::EGGLAYER) {
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

		// link all muscles to joints:
		resolveMuscleLinkage();

		// now decode the neural network:
		initializeNeuralNetwork();
		decodeDeferredGenes();
		// link nerves to sensors and motors:
		resolveNerveLinkage();
		// commit neuron properties:
		commitNeurons();

		// clean up:
		cleanUp();

		return false;
	}
	unsigned nCrtBranches = activeSet_.size();
	for (unsigned i=0; i<nCrtBranches; i++) {
#ifdef ENABLE_START_MARKER_GENES
		if (activeSet_[i].second.crtGenomePos == activeSet_[i].second.startGenomePos) {
			// move forward until we hit a start marker
			auto &c1 = bug_->genome_.first.genes;
			auto &c2 = bug_->genome_.second.genes;
			auto &offs = activeSet_[i].second.crtGenomePos;
			while ((c1.size() > offs && c1[offs].type != gene_type::START_MARKER)
				|| (c2.size() > offs && c2[offs].type != gene_type::START_MARKER)) {
				activeSet_[i].second.crtGenomePos++;
				// did we hit a marker?
				if ((c1.size() > offs && c1[offs].type == gene_type::START_MARKER)
					|| (c2.size() > offs && c2[offs].type == gene_type::START_MARKER))
					break;
			}
		}
#endif
		BodyPart* p = activeSet_[i].first;
		unsigned offset = activeSet_[i].second.crtGenomePos++;
		Gene *g1 = nullptr, *g2 = nullptr;
		if (offset < bug_->genome_.first.genes.size())
			g1 = &bug_->genome_.first.genes[offset];
		if (offset < bug_->genome_.second.genes.size())
			g2 = &bug_->genome_.second.genes[offset];
		bool reachedTheEnd = !g1 && !g2;
		if (reachedTheEnd
				|| (g1 && g1->type == gene_type::STOP)
				|| (g2 && g2->type == gene_type::STOP)) {
			// so much for this development path;
			// grow body parts from all segments now
			for (unsigned k=0; k<BodyPart::MAX_CHILDREN; k++)
				growBodyPart(p, k, activeSet_[i].second.hyperPositions[k],
						activeSet_[i].second.startGenomePos + activeSet_[i].second.offsets[k]);
			// decode joint genes if such is the case:
			auto it = mapJointOffsets_.find(p);
			if (it != mapJointOffsets_.end()) {
				Joint* joint = it->second.first;
				int jOffset = it->second.second.hasValue() ? it->second.second : 0;
				activeSet_.push_back(std::make_pair(joint, activeSet_[i].second.startGenomePos + jOffset));
			}
			// and remove this branch:
			activeSet_.erase(activeSet_.begin()+i);
			i--, nCrtBranches--;
			continue;
		}

		// now decode the genes:
		if (g1)
			decodeGene(*g1, p, &activeSet_[i].second, true);
		if (g2)
			decodeGene(*g2, p, &activeSet_[i].second, true);

		int skipCount = 0;
		if (g1 && g1->type == gene_type::SKIP) {
			int depth = p->getDepth();
			if (depth <= g1->data.gene_skip.maxDepth && depth >= g1->data.gene_skip.minDepth)
				skipCount += g1->data.gene_skip.count;
		}
		if (g2 && g2->type == gene_type::SKIP) {
			int depth = p->getDepth();
			if (depth <= g2->data.gene_skip.maxDepth && depth >= g2->data.gene_skip.minDepth) {
				if (skipCount)
					skipCount = (skipCount + g2->data.gene_skip.count) / 2;
				else
					skipCount = g2->data.gene_skip.count;
			}
		}
		activeSet_[i].second.crtGenomePos += skipCount;
	}
	return true;
}

void Ribosome::growBodyPart(BodyPart* parent, unsigned attachmentSegment, glm::vec4 hyperPosition, unsigned genomeOffset) {
	// grow only works on bones and torso
	if (parent->getType() != BodyPartType::BONE && parent->getType() != BodyPartType::TORSO)
		return;
	// determine the body part type to grow from the hyperPosition
	BodyPartType partTypes[2][2][2][2] = {
		/* W- */ {
			/* Z- */ {
				/* Y- */ {
					/* X- */ BodyPartType::BONE, /* X+ */ BodyPartType::INVALID
				},
				/* Y+ */ {
					/* X- */ BodyPartType::GRIPPER, /* X+ */ BodyPartType::MOUTH
				},
			},
			/* Z+ */ {
				/* Y- */ {
					/* X- */ BodyPartType::INVALID, /* X+ */ BodyPartType::INVALID
				},
				/* Y+ */ {
					/* X- */ BodyPartType::MUSCLE, /* X+ */ BodyPartType::EGGLAYER
				},
			},
		},
		/* W+ */ {
			/* Z- */ {
				/* Y- */ {
					/* X- */ BodyPartType::INVALID, /* X+ */ BodyPartType::INVALID
				},
				/* Y+ */ {
					/* X- */ BodyPartType::SENSOR_PROXIMITY, /* X+ */ BodyPartType::INVALID
				},
			},
			/* Z+ */ {
				/* Y- */ {
					/* X- */ BodyPartType::INVALID, /* X+ */ BodyPartType::INVALID
				},
				/* Y+ */ {
					/* X- */ BodyPartType::SENSOR_COMPASS, /* X+ */ BodyPartType::SENSOR_SIGHT
				},
			},
		}
	};
	// if any one axis is zero, we cannot determine the part type and none is grown
	if (hyperPosition.x * hyperPosition.y * hyperPosition.z * hyperPosition.w == 0)
		return;
	BodyPartType newBodyPartType = partTypes[hyperPosition.w > 0][hyperPosition.z > 0][hyperPosition.y > 0][hyperPosition.x > 0];
	if (newBodyPartType == BodyPartType::INVALID)
		return;

	// TODO Auto-generate body-part-sensors in joints & grippers and other parts that may have useful info

	float angle = attachmentSegment * 2*PI / BodyPart::MAX_CHILDREN;

	// The child's attachment point relative to the parent's center is computed from the angle of the current segment,
	// by casting a ray from the parent's origin in the specified angle (which is relative to the parent's orientation)
	// until it touches an edge of the parent. That point is used as attachment of the new part.

	Joint* upstreamJoint = nullptr;
	bool useUpstreamJoint = partMustGenerateJoint(newBodyPartType);
	if (useUpstreamJoint) {
		// we cannot grow this part directly onto its parent, they must be connected by a joint
		upstreamJoint = new Joint();
		parent->add(upstreamJoint, angle);

		// set part to point to the joint's node, since that's where the actual part will be attached:
		parent = upstreamJoint;
		// recompute coordinates in joint's space:
		angle = 0;
	}

	BodyPart* bp = nullptr;
	IMotor* pMotor = nullptr;
	ISensor* pSensor = nullptr;
	switch (newBodyPartType) {
	case BodyPartType::BONE:
		bp = new Bone();
		break;
	case BodyPartType::GRIPPER: {
		Gripper* gr = new Gripper();
		pMotor = gr;
		bp = gr;
		break;
	}
	case BodyPartType::MUSCLE: {
		// muscle must be linked to the nearest joint - or one towards which it's oriented if equidistant
		// linkage is postponed until before commit when all parts are in place (muscle may be created before joint)
		Muscle* m = new Muscle();
		muscles_.push_back(m);
		pMotor = m;
		bp = m;
		break;
	}
	case BodyPartType::MOUTH: {
		Mouth* m = new Mouth();
		bp = m;
		break;
	}
	case BodyPartType::SENSOR_COMPASS:
		// bp = new sensortype?(part->bodyPart, PhysicsProperties(offset, angle));
		break;
//	case BodyPartType::SENSOR_DIRECTION:
		// bp = new sensortype?(part->bodyPart, PhysicsProperties(offset, angle));
//		break;
	case BodyPartType::SENSOR_PROXIMITY: {
		Nose* n = new Nose();
		pSensor = n;
		bp = n;
		break;
	}
	case BodyPartType::SENSOR_SIGHT:
		// bp = new sensortype?(part->bodyPart, PhysicsProperties(offset, angle));
		break;
	case BodyPartType::EGGLAYER: {
		EggLayer* e = new EggLayer();
		pMotor = e;
		bug_->eggLayers_.push_back(e);
		bp = e;
		break;
	}
	default:
		ERROR("unhandled gene part type: "<<(uint)newBodyPartType);
		break;
	}
	if (!bp)
		return;

	if (useUpstreamJoint) {
		// add joint mapping to this part:
		mapJointOffsets_[bp] = std::make_pair(upstreamJoint, CummulativeValue());
	}

	parent->add(bp, angle);

	// this must happen AFTER the part is added to its parent:
	if (pMotor)
		addMotor(pMotor, bp);
	if (pSensor)
		addSensor(pSensor);

	// start a new development path from the new part:
	activeSet_.push_back(std::make_pair(bp, genomeOffset));
}

void Ribosome::addMotor(IMotor* motor, BodyPart* part) {
	motors_.push_back(motor);
	for (unsigned i=0; i<motor->getInputCount(); i++) {
		int lineId = nMotorLines_++;
		part->addMotorLine(lineId);
	}
}
void Ribosome::addSensor(ISensor* sensor) {
	sensors_.push_back(sensor);
}

bool Ribosome::hasNeuron(int virtualIndex, bool physical) {
	 bool hasVirtual = mapNeurons_.find(virtualIndex) != mapNeurons_.end();
	 if (physical)
		 return hasVirtual && bug_->neuralNet_ && mapNeurons_[virtualIndex].index < bug_->neuralNet_->neurons.size();
	 else
		 return hasVirtual;
}

void Ribosome::checkAndAddNeuronMapping(int virtualIndex) {
	if (!hasNeuron(virtualIndex, false)) {
		int realIndex = mapNeurons_.size();
		mapNeurons_[virtualIndex] = NeuronInfo(realIndex);
	}
}

void Ribosome::decodeGene(Gene const& g, BodyPart* part, GrowthData *growthData, bool deferNeural) {
	switch (g.type) {
	case gene_type::NO_OP:
		break;
#ifdef ENABLE_START_MARKER_GENES
	case gene_type::START_MARKER:
		break;
#endif
	case gene_type::SKIP:
		break;
	case gene_type::STOP:
		break;
	case gene_type::PROTEIN:
		decodeProtein(g.data.gene_protein, part, growthData);
		break;
	case gene_type::OFFSET:
		decodeOffset(g.data.gene_offset, part, growthData);
		break;
	case gene_type::JOINT_OFFSET:
		decodeJointOffset(g.data.gene_joint_offset, part);
		break;
	case gene_type::PART_ATTRIBUTE:
		decodePartAttrib(g.data.gene_attribute, part);
		break;
	case gene_type::BODY_ATTRIBUTE:
		bug_->mapBodyAttributes_[g.data.gene_body_attribute.attribute]->changeAbs(g.data.gene_body_attribute.value);
		break;
	case gene_type::SYNAPSE:
		// only from depth 0 (torso) must the neural genes be taken into account
		if (!part || part->getType() == BodyPartType::TORSO) {
			decodeSynapse(g.data.gene_synapse);
		}
		break;
	case gene_type::NEURON_OUTPUT_COORD:
		if (!part || part->getType() == BodyPartType::TORSO) {
			if (deferNeural)
				neuralGenes_.push_back(&g);
			else
				decodeNeuronOutputCoord(g.data.gene_neuron_output);
		}
		break;
	case gene_type::NEURON_INPUT_COORD:
		if (!part || part->getType() == BodyPartType::TORSO) {
			if (deferNeural)
				neuralGenes_.push_back(&g);
			else
				decodeNeuronInputCoord(g.data.gene_neuron_input);
		}
		break;
	case gene_type::TRANSFER_FUNC:
		if (!part || part->getType() == BodyPartType::TORSO) {
			if (deferNeural)
				neuralGenes_.push_back(&g);
			else
				decodeTransferFn(g.data.gene_transfer_function);
		}
		break;
	case gene_type::NEURAL_BIAS:
		if (!part || part->getType() == BodyPartType::TORSO) {
			if (deferNeural)
				neuralGenes_.push_back(&g);
			else
				decodeNeuralBias(g.data.gene_neural_constant);
		}
		break;
	case gene_type::NEURAL_PARAM:
		if (!part || part->getType() == BodyPartType::TORSO) {
			if (deferNeural)
				neuralGenes_.push_back(&g);
			else
				decodeNeuralParam(g.data.gene_neural_param);
		}
		break;
	default:
		ERROR("Unhandled gene type : " << (uint)g.type);
	}
}

bool Ribosome::partMustGenerateJoint(BodyPartType part_type) {
	switch (part_type) {
	case BodyPartType::BONE:
	case BodyPartType::GRIPPER:
		return true;
	default:
		return false;
	}
}

void Ribosome::decodeProtein(GeneProtein const& g, BodyPart* part, GrowthData *growthData) {
	int crtDepth = part->getDepth();
	if (crtDepth < g.minDepth || crtDepth > g.maxDepth)
		return;
	uint segment = clamp<int>(g.targetSegment, 0, BodyPart::MAX_CHILDREN-1);
	glm::vec4 &pos = growthData->hyperPositions[segment];
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

void Ribosome::decodeOffset(GeneOffset const& g, BodyPart *part, GrowthData *growthData) {
	int crtDepth = part->getDepth();
	if (crtDepth < g.minDepth || crtDepth > g.maxDepth)
		return;
	uint segment = clamp<int>(g.targetSegment, 0, BodyPart::MAX_CHILDREN-1);
	growthData->offsets[segment].changeAbs(g.offset);
}

void Ribosome::decodeJointOffset(GeneJointOffset const& g, BodyPart* part) {
	int crtDepth = part->getDepth();
	if (crtDepth < g.minDepth || crtDepth > g.maxDepth)
		return;
	if (mapJointOffsets_.find(part) != mapJointOffsets_.end())
		mapJointOffsets_[part].second.changeAbs(g.offset);
}

void Ribosome::decodePartAttrib(GeneAttribute const& g, BodyPart* part) {
	int depth = part->getDepth();
	if (depth >= g.minDepth && depth <= g.maxDepth)
	{
		CummulativeValue* pAttrib = part->getAttribute(g.attribute, g.attribIndex);
		if (pAttrib)
			pAttrib->changeAbs(g.value);
	}
}

void Ribosome::decodeSynapse(GeneSynapse const& g) {
	// the number of neurons is derived from the synapse values
	checkAndAddNeuronMapping(g.from);
	checkAndAddNeuronMapping(g.to);
	uint64_t key = synKey(g.from, g.to);
	assert(!std::isnan(g.weight.value));
	mapSynapses_[key].weight.changeAbs(g.weight);
	mapSynapses_[key].priority.changeAbs(g.priority);
}

void Ribosome::decodeTransferFn(GeneTransferFunction const& g) {
	if (hasNeuron(g.targetNeuron, false))
		mapNeurons_[g.targetNeuron].transfer.changeAbs(g.functionID);
}

void Ribosome::decodeNeuralBias(GeneNeuralBias const& g) {
	assert(!std::isnan(g.value.value));
	if (hasNeuron(g.targetNeuron, false))
		mapNeurons_[g.targetNeuron].bias.changeAbs(g.value);
}

void Ribosome::decodeNeuralParam(GeneNeuralParam const& g) {
	assert(!std::isnan(g.value.value));
	if (hasNeuron(g.targetNeuron, false))
		mapNeurons_[g.targetNeuron].param.changeAbs(g.value);
}

void Ribosome::decodeNeuronOutputCoord(GeneNeuronOutputCoord const& g) {
	checkAndAddNeuronMapping(g.srcNeuronVirtIndex);
	mapNeurons_[g.srcNeuronVirtIndex].outputVMSCoord.changeAbs(g.outCoord);
	// add this neuron into the outputNeurons_ set:
	outputNeurons_.insert(g.srcNeuronVirtIndex);
}

void Ribosome::decodeNeuronInputCoord(GeneNeuronInputCoord const& g) {
	checkAndAddNeuronMapping(g.destNeuronVirtIndex);
	mapNeurons_[g.destNeuronVirtIndex].inputVMSCoord.changeAbs(g.inCoord);
	// add this neuron into the inputNeurons_ set:
	inputNeurons_.insert(g.destNeuronVirtIndex);
}

void Ribosome::createSynapse(int from, int to, SynapseInfo const& info) {
	assertDbg(hasNeuron(from, true));	// should be there, since synapses dictate neurons
	assertDbg(hasNeuron(to, true));

	OutputSocket* pFrom = &bug_->neuralNet_->neurons[mapNeurons_[from].index]->output;
	Neuron* pTo = bug_->neuralNet_->neurons[mapNeurons_[to].index];

	InputSocket* i = new InputSocket(pTo, info.weight);
	pTo->addInput(std::unique_ptr<InputSocket>(i), info.priority);
	pFrom->addTarget(i);
}

// returns -1 if none found
template <typename T>
int Ribosome::getVMSNearestNerveIndex(std::vector<std::pair<T, float>> const& nerves, float matchCoord) {
	if (nerves.size() == 0)
		return -1;
	// binary-search the nearest output neuron:
	unsigned small = 0, big = nerves.size()-1;
	while (small != big) {
		unsigned pivot = (big-small) / 2 + small;
		if (matchCoord > nerves[pivot].second) { // look into the big interval
			if (pivot < nerves.size()-1) {	// there are greater
				float crtDelta = matchCoord - nerves[pivot].second;
				float nextDelta = matchCoord - nerves[pivot+1].second;
				if (fabs(crtDelta) > fabs(nextDelta)) {
					// move to the greater interval:
					if (small != pivot)
						small = pivot;
					else
						small = pivot+1;
				} else	// this is the closest we can get
					return pivot;
			} else // this is the closest we can get
				return pivot;
		} else if (matchCoord < nerves[pivot].second) { // look into the small interval
			if (pivot > 0) {	// there are smaller
				float crtDelta = matchCoord - nerves[pivot].second;
				float prevDelta = matchCoord - nerves[pivot-1].second;
				if (fabs(crtDelta) > fabs(prevDelta)) {
					// move to the small interval
					if (big != pivot)
						big = pivot;
					else
						big = pivot-1;
				} else	// this is the closest we can get
					return pivot;
			} else	// this is the closest we can get
				return pivot;
		} else
			return pivot;	// perfect match!
	}
	return small;
}

void Ribosome::linkMotorNerves(std::vector<InputOutputNerve<Neuron*>> const& orderedOutputNeurons_,
							   std::vector<InputOutputNerve<InputSocket*>> const& orderedMotorInputs_) {
	bug_->motorLines_.clear();
	// motors are matched 1:1 with the nearest output nerves from the neural network, in the direction from motor nerve to output nerve.
	for (unsigned i = 0; i < orderedMotorInputs_.size(); i++) {
		float motorCoord = orderedMotorInputs_[i].second;
		if (motorCoord == 0)
			continue;
		int neuronIndex = getVMSNearestNerveIndex(orderedOutputNeurons_, motorCoord);
		if (neuronIndex >= 0) {
			// link this motor to this neuron
			orderedOutputNeurons_[neuronIndex].first->output.addTarget(orderedMotorInputs_[i].first);
			// add mapping for this motor line in bug:
			int nerveLineId = mapInputNerves_[orderedMotorInputs_[i].first];
			bug_->motorLines_[nerveLineId] = std::make_pair(orderedMotorInputs_[i].first, &orderedOutputNeurons_[neuronIndex].first->output);

#ifdef DEBUG
			if (false) {
				LOGLN("LinkMotorNerve: virtN[" << mapNeuronVirtIndex_[orderedOutputNeurons_[neuronIndex].first] << "] to "
						<< mapSockMotorInfo[orderedMotorInputs_[i].first].first << "@@"
						<< mapSockMotorInfo[orderedMotorInputs_[i].first].second
						<< " {lineId:" << nerveLineId << "}");
			}
#endif
		}
	}
}

void Ribosome::linkSensorNerves(std::vector<InputOutputNerve<Neuron*>> const& orderedInputNeurons_,
						  	    std::vector<InputOutputNerve<OutputSocket*>> orderedSensorOutputs_) {
	// sensors are matched n:m with nearest input nerves in two passes:
	// 1. all input nerves are connected to the nearest sensor nerves
	// 2. all unconnected sensor nerves are connected to the nearest input nerves

	// stage 1:
	for (auto &inerve : orderedInputNeurons_) {
		int sensorSocketIndex = getVMSNearestNerveIndex(orderedSensorOutputs_, inerve.second);
		if (sensorSocketIndex >= 0) {
			std::unique_ptr<InputSocket> sock = std::unique_ptr<InputSocket>(new InputSocket(inerve.first, 1.f));
			orderedSensorOutputs_[sensorSocketIndex].first->addTarget(sock.get());
			inerve.first->addInput(std::move(sock), 0);

#ifdef DEBUG
//			if (true) {
//				LOGLN("LinkSensorNerve: virtN[" << mapNeuronVirtIndex_[orderedOutputNeurons_[neuronIndex].first] << "] to "
//						<< mapSockMotorInfo[orderedMotorInputs_[i].first].first << "@@"
//						<< mapSockMotorInfo[orderedMotorInputs_[i].first].second
//						<< " {lineId:" << nerveLineId << "}");
//			}
#endif

			orderedSensorOutputs_.erase(orderedSensorOutputs_.begin() + sensorSocketIndex);
		}
	}

	// stage 2:
	for (auto &sensor : orderedSensorOutputs_) {
		if (sensor.second == 0)
			continue;
		int nerveIndex = getVMSNearestNerveIndex(orderedInputNeurons_, sensor.second);
		if (nerveIndex >= 0) {
			Neuron* neuron = orderedInputNeurons_[nerveIndex].first;
			std::unique_ptr<InputSocket> sock = std::unique_ptr<InputSocket>(new InputSocket(neuron, 1.f));
			sensor.first->addTarget(sock.get());
			neuron->addInput(std::move(sock), 0);
		}
	}
}

void Ribosome::resolveNerveLinkage() {
	// build the motor input nerves vector:
	std::vector<InputOutputNerve<InputSocket*>> motorInputs;
	for (unsigned i=0; i<motors_.size(); i++) {
		for (unsigned j=0; j<motors_[i]->getInputCount(); j++) {
			mapInputNerves_[motors_[i]->getInputSocket(j)] = motorInputs.size();
			motorInputs.push_back(std::make_pair(motors_[i]->getInputSocket(j), motors_[i]->getInputVMSCoord(j)));
#ifdef DEBUG
			mapSockMotorInfo[motors_[i]->getInputSocket(j)] = std::make_pair(motors_[i]->getMotorDebugName(), j);
#endif
		}
	}
	// build the sensor output nerves vector:
	std::vector<InputOutputNerve<OutputSocket*>> sensorOutputs;
	for (unsigned i=0; i<sensors_.size(); i++) {
		for (unsigned j=0; j<sensors_[i]->getOutputCount(); j++)
			sensorOutputs.push_back(std::make_pair(sensors_[i]->getOutputSocket(j), sensors_[i]->getOutputVMSCoord(j)));
	}
	// build the neuron vectors:
	std::vector<InputOutputNerve<Neuron*>> inputNeurons;
	for (int i : inputNeurons_) {
		if (!hasNeuron(i, true))
			continue; // this neuron doesn't actually exist because it doesn't participate in any synapses
		Neuron* neuron = bug_->neuralNet_->neurons[mapNeurons_[i].index];
		float vmsCoord = mapNeurons_[i].inputVMSCoord;
		inputNeurons.push_back(std::make_pair(neuron, vmsCoord));
	}
	std::vector<InputOutputNerve<Neuron*>> outputNeurons;
	for (int i : outputNeurons_) {
		if (!hasNeuron(i, true))
			continue; // this neuron doesn't actually exist because it doesn't participate in any synapses
		Neuron* neuron = bug_->neuralNet_->neurons[mapNeurons_[i].index];
		float vmsCoord = mapNeurons_[i].outputVMSCoord;
		outputNeurons.push_back(std::make_pair(neuron, vmsCoord));
	}
	// sort the input/output nerves by their VMS coords, smallest to greatest:
	sortNervesByVMSCoord(motorInputs);
	sortNervesByVMSCoord(sensorOutputs);
	sortNervesByVMSCoord(outputNeurons);
	sortNervesByVMSCoord(inputNeurons);

	// link nerves to motors/sensors:
	linkMotorNerves(outputNeurons, motorInputs);
	linkSensorNerves(inputNeurons, sensorOutputs);

	motors_.clear();
	sensors_.clear();
	inputNeurons_.clear();
	outputNeurons_.clear();
}

void Ribosome::commitNeurons() {
	for (auto &n : bug_->neuralNet_->neurons)
		n->commitInputs();
}

Joint* Ribosome::findNearestJoint(Muscle* m, int dir) {
	assertDbg(m->getParent() && "muscle should have a parent!");
	int nChildren = m->getParent()->getChildrenCount();
	std::vector<BodyPart*> bp;
	bp.reserve(nChildren);
	for (int i=0; i<nChildren; i++)
		bp.push_back(m->getParent()->getChild(i));
	std::sort(bp.begin(), bp.end(), [] (BodyPart* left, BodyPart* right) -> bool {
		return left->getAttachmentAngle() < right->getAttachmentAngle();
	});

	int mIndex = -1;
	for (int i=0; i<nChildren; i++) {
		if (m->getParent()->getChild(i) == m) {
			mIndex = i;
			break;
		}
	}
	assertDbg(mIndex >= 0 && "muscle should have been found in parent!");
	int index = mIndex;
	do {
		if (dir > 0)
			index = circularNext(index, nChildren);
		else
			index = circularPrev(index, nChildren);

		if (m->getParent()->getChild(index)->getType() == BodyPartType::JOINT)
			return dynamic_cast<Joint*>(m->getParent()->getChild(index));
	} while (index != mIndex);
	return nullptr;
}

void Ribosome::resolveMuscleLinkage() {
	for (Muscle* m : muscles_) {
		Joint* jNeg = findNearestJoint(m, -1);
		Joint* jPos = findNearestJoint(m, +1);
		if (!jNeg && !jPos)
			continue;
		// default to the joint on the negative side and only select the positive one if more appropriate:
		Joint* targetJoint = jNeg;
		if (jNeg != jPos) {
			float negDelta = absAngleDiff(jNeg->getAttachmentAngle(), m->getAttachmentAngle());
			float posDelta = absAngleDiff(jPos->getAttachmentAngle(), m->getAttachmentAngle());
			if (posDelta < negDelta) {
				targetJoint = jPos;
			} else if (posDelta == negDelta) {
				// angle differences are equal, choose the one towards which the muscle is oriented
				if (m->getLocalRotation() > 0) {
					targetJoint = jPos;
				}
			}
		}
		m->setJoint(targetJoint, angleDiff(m->getAttachmentAngle(), targetJoint->getAttachmentAngle()) > 0 ? -1 : +1);
	}
	muscles_.clear();
}
