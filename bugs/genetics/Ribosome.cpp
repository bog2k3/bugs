#include "Ribosome.h"

#include "../entities/Bug.h"
#include "../body-parts/BodyConst.h"
#include "../body-parts/BodyCell.h"
#include "../body-parts/BodyPart.h"
#include "../body-parts/Torso.h"
#include "../body-parts/Bone.h"
#include "../body-parts/Gripper.h"
#include "../body-parts/ZygoteShell.h"
#include "../body-parts/Muscle.h"
#include "../body-parts/Mouth.h"
#include "../body-parts/EggLayer.h"
#include "../body-parts/sensors/Nose.h"
#include "../body-parts/JointPivot.h"
#include "../body-parts/FatCell.h"
#include "../neuralnet/functions.h"
#include "../neuralnet/Network.h"
#include "../neuralnet/Neuron.h"
#include "../neuralnet/OutputSocket.h"
#include "../neuralnet/functions.h"
#include "../neuralnet/InputSocket.h"
#include "Gene.h"
#include "Genome.h"
#include "GeneDefinitions.h"
#include "CumulativeValue.h"
#include "ProteinHyperspace.h"

#include <boglfw/math/math3D.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/rand.h>
#include <boglfw/utils/log.h>

#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/renderOpenGL/GLText.h>

#include <glm/gtx/transform.hpp>

#include <utility>
#include <algorithm>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static std::map<BodyPartType, float (*)(BodyCell const&)> mapDensityFunctions {
	{ BodyPartType::BONE,				Bone::getDensity },
	{ BodyPartType::EGGLAYER,			EggLayer::getDensity },
	{ BodyPartType::FAT, 				FatCell::getDensity },
	{ BodyPartType::GRIPPER, 			Gripper::getDensity },
	{ BodyPartType::MOUTH, 				Mouth::getDensity },
//	{ BodyPartType::SENSOR_COMPASS, 	SensorCompass::getDensity },
	{ BodyPartType::SENSOR_PROXIMITY,	Nose::getDensity },
//	{ BodyPartType::SENSOR_SIGHT, 		Eye::getDensity },
};

static std::map<BodyPartType, float (*)(BodyCell const& cell, float angle)> mapRadiusFunctions {
	{ BodyPartType::BONE,				Bone::getRadius },
	{ BodyPartType::EGGLAYER,			EggLayer::getRadius },
	{ BodyPartType::FAT, 				FatCell::getRadius },
	{ BodyPartType::GRIPPER, 			Gripper::getRadius },
	{ BodyPartType::MOUTH, 				Mouth::getRadius },
//	{ BodyPartType::SENSOR_COMPASS, 	SensorCompass::getRadius },
	{ BodyPartType::SENSOR_PROXIMITY,	Nose::getRadius },
//	{ BodyPartType::SENSOR_SIGHT, 		Eye::getRadius },
};

Ribosome::Ribosome(Bug* bug)
	: bug_{bug}
{
	// there are no default body parts; they either get created by the genes, or the embryo
	// is discarded at the end of development if it lacks critical parts such as mouth or egg-layer

	float initialSize = bug->zygoteShell_->getMass() / BodyConst::FatDensity;	// because unspecialized cells have the density of fat
	BodyCell* initialCell = new BodyCell(initialSize, glm::vec2(0, 0), 0, false, false);
	cells_.push_back(initialCell);

	// start decoding with root body part at offset 0 in the genome:
	activeSet_.push_back(std::make_pair(initialCell, 0));
}

Ribosome::~Ribosome() {
	cleanUp();
}

void Ribosome::cleanUp() {
	neuralGenes_.clear();
	activeSet_.clear();
//	mapNeurons_.clear();
//	mapSynapses_.clear();
//	outputNeurons_.clear();
//	inputNeurons_.clear();
	motors_.clear();
	sensors_.clear();
	mapInputNerves_.clear();

	throw std::runtime_error("make sure this is complete");
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
//	return abs(d1) < abs(d2);
//}

void Ribosome::initializeNeuralNetwork() {
	// create and initialize the neural network:
	bug_->neuralNet_ = new NeuralNet();
//	bug_->neuralNet_->neurons.reserve(mapNeurons_.size());
//	for (uint i=0; i<mapNeurons_.size(); i++) {
//		bug_->neuralNet_->neurons.push_back(new Neuron());
//	}
//#ifdef DEBUG
//	for (auto const& it : mapNeurons_) {
//		mapNeuronVirtIndex_[bug_->neuralNet_->neurons[it.second.index]] = it.first;
//		if (false) {
//			LOGLN("Neuron MAPPING: " << it.first << "(v) -> " << it.second.index << "(r)" << "\t" << bug_->neuralNet_->neurons[it.second.index]);
//		}
//	}
//#endif
	throw std::runtime_error("Not implemented!");
}

void Ribosome::decodeDeferredGenes() {
	// create all synapses
//	for (auto s : mapSynapses_) {
//		int32_t from = s.first >> 32;
//		int32_t to = (s.first) & 0xFFFFFFFF;
//		createSynapse(from, to, s.second);
//	}
//	// now decode the deferred neural genes (neuron properties):
//	for (auto &g : neuralGenes_)
//		decodeGene(*g, nullptr, nullptr, false);
//	// apply all neuron properties
//	for (auto &n : mapNeurons_) {
//		if (n.second.transfer.hasValue()) {
//			int funcIndex = clamp((int)n.second.transfer.get(),
//					(int)transferFuncNames::FN_ONE,
//					(int)transferFuncNames::FN_MAXCOUNT-1);
//			bug_->neuralNet_->neurons[n.second.index]->setTranferFunction((transferFuncNames)funcIndex);
//		}
//		if (n.second.bias.hasValue())
//			bug_->neuralNet_->neurons[n.second.index]->inputBias = n.second.bias;
//		if (n.second.param.hasValue())
//			bug_->neuralNet_->neurons[n.second.index]->neuralParam = n.second.param;
//	}
	throw std::runtime_error("Not implemented!");
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
		// finished division and specialization

		// check if critical body parts exist (at least a mouth and egg-layer)
		bool hasMouth = false, hasEggLayer = false;
		specializeCells(hasMouth, hasEggLayer);

		if (!hasMouth || !hasEggLayer) {
			// here mark the embryo as dead and return
			bug_->isAlive_ = false;
			cleanUp();
			return false;
		}

		// TODO must now decode neural genes from neuralGenes_ set and body attribute genes from bodyAttribGenes_ set

		// link all muscles to joints:
		resolveMuscleLinkage();

		// now decode the neural network:
		initializeNeuralNetwork();
		decodeDeferredGenes();
		// link nerves to sensors and motors:
		resolveNerveLinkage();
		// commit neuron properties:
//		commitNeurons();
		throw std::runtime_error("Not implemented!");

		// clean up:
		cleanUp();

		return false;
	}
	unsigned nCrtBranches = activeSet_.size();
	for (unsigned i=0; i<nCrtBranches; i++) {
		BodyCell* cell = activeSet_[i].first;
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
			// decide if cell will divide or specialize
			if (cell->mapDivisionParams_[GENE_DIVISION_AFFINITY] > 0.f) {
				// check if division will create pivot joint, and if so, we need to subtract the mass required to make the joint and muscles
				if (cell->mapJointAttribs_[GENE_JOINT_ATTR_TYPE] > 0.f) {
					float cellMass = cell->density() * cell->size();
					float jointMass = cellMass * cell->mapJointAttribs_[GENE_JOINT_ATTR_MASS_RATIO].clamp(
							BodyConst::MinJointMassRatio, BodyConst::MaxJointMassRatio);
					cell->muscleMassLeft_ = (cellMass-jointMass) * cell->mapLeftMuscleAttribs_[GENE_MUSCLE_ATTR_MASS_RATIO].clamp(
							BodyConst::MinMuscleMassRatio, BodyConst::MaxMuscleMassRatio);
					cell->muscleMassRight_ = (cellMass-jointMass) * cell->mapRightMuscleAttribs_[GENE_MUSCLE_ATTR_MASS_RATIO].clamp(
							BodyConst::MinMuscleMassRatio, BodyConst::MaxMuscleMassRatio);
					cell->size_ *= (cellMass - jointMass - cell->muscleMassLeft_ - cell->muscleMassRight_) / cellMass;
					cell->setJointSize(jointMass / cellMass * cell->size_);
				}
				// divide
				auto pair = cell->divide();
				cells_.push_back(pair.first);
				cells_.push_back(pair.second);
				DecodeContext leftCtx(activeSet_[i].second.startGenomePos + activeSet_[i].second.childOffsets[0]);
				DecodeContext rightCtx(activeSet_[i].second.startGenomePos + activeSet_[i].second.childOffsets[1]);
				activeSet_.push_back({pair.first, leftCtx});
				activeSet_.push_back({pair.second, rightCtx});
			}
//			for (unsigned k=0; k<BodyPart::MAX_CHILDREN; k++)
//				growBodyPart(p, k, activeSet_[i].second.hyperPositions[k],
//						activeSet_[i].second.startGenomePos + activeSet_[i].second.offsets[k]);
			// decode joint genes if such is the case:
//			auto it = mapJointOffsets_.find(p);
//			if (it != mapJointOffsets_.end()) {
//				JointPivot* joint = it->second.first;
//				int jOffset = it->second.second.hasValue() ? it->second.second : 0;
//				activeSet_.push_back(std::make_pair(joint, activeSet_[i].second.startGenomePos + jOffset));
//			}
			// and remove this branch:
			activeSet_.erase(activeSet_.begin()+i);
			i--, nCrtBranches--;
			continue;
		}

		// now decode the genes:
		if (g1 && geneQualifies(*g1, *cell))
			decodeGene(*g1, *cell, activeSet_[i].second, true);
		if (g2 && geneQualifies(*g2, *cell))
			decodeGene(*g2, *cell, activeSet_[i].second, true);

		int skipCount = 0;
		if (g1 && g1->type == gene_type::SKIP) {
			//int depth = p->getDepth();
			//if (depth <= g1->data.gene_skip.maxDepth && depth >= g1->data.gene_skip.minDepth)
			if (geneQualifies(*g1, *cell))
				skipCount += g1->data.gene_skip.count;
		}
		if (g2 && g2->type == gene_type::SKIP) {
			if (geneQualifies(*g2, *cell)) {
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

BodyPartType Ribosome::specializationType(BodyCell const& c) const {
	return proteinHyperspace
			[c.proteinValues_.x > 0 ? 1 : 0]
			[c.proteinValues_.y > 0 ? 1 : 0]
			[c.proteinValues_.z > 0 ? 1 : 0]
			[c.proteinValues_.w > 0 ? 1 : 0];
}

void Ribosome::specializeCells(bool &hasMouth, bool &hasEggLayer) {
	// update cells' density, then set radiusFn in all cells, then call fixOverlap on each of them
	for (auto c : cells_) {
		if (!c->isActive())
			continue;
		updateCellDensity(*c);
		switch (specializationType(*c)) {
		case BodyPartType::MOUTH:
			hasMouth = true;
			break;
		case BodyPartType::EGGLAYER:
			hasEggLayer = true;
			break;
		case BodyPartType::BONE:
			break;
		case BodyPartType::FAT:
			break;
		case BodyPartType::GRIPPER:
			break;
		case BodyPartType::SENSOR_COMPASS:
			break;
		case BodyPartType::SENSOR_PROXIMITY:
			break;
		case BodyPartType::SENSOR_SIGHT:
			break;
		default:
			throw std::runtime_error("invalid specialization type!");
		};
	}
}

// call this before instantiating the body part in order to update to correct density and size
void Ribosome::updateCellDensity(BodyCell &cell) {
	auto fn = mapDensityFunctions[specializationType(cell)];	// undefined function and map -> implement with static methods for density in bodyparts
	auto oldDensity = cell.density_;
	cell.density_ = fn(cell);
	// must adjust cell size to conserve mass
	cell.size_ *= oldDensity / cell.density_;
	// adjust the cell's shape:
	cell.radiusFn = mapRadiusFunctions[specializationType(cell)];
}

/*void Ribosome::growBodyPart(BodyPart* parent, unsigned attachmentSegment, glm::vec4 hyperPosition, unsigned genomeOffset) {
	// grow only works on bones and torso
	if (parent->getType() != BodyPartType::BONE && parent->getType() != BodyPartType::TORSO)
		return;
	// determine the body part type to grow from the hyperPosition

	// if any one axis is zero, we cannot determine the part type and none is grown
	if (hyperPosition.x * hyperPosition.y * hyperPosition.z * hyperPosition.w == 0)
		return;
	BodyPartType newBodyPartType = proteinHyperspace[hyperPosition.w > 0][hyperPosition.z > 0][hyperPosition.y > 0][hyperPosition.x > 0];
	if (newBodyPartType == BodyPartType::INVALID)
		return;

	// TODO Auto-generate body-part-sensors in joints & grippers and other parts that may have useful info

//	float angle = attachmentSegment * 2*PI / BodyPart::MAX_CHILDREN;

	// The child's attachment point relative to the parent's center is computed from the angle of the current segment,
	// by casting a ray from the parent's origin in the specified angle (which is relative to the parent's orientation)
	// until it touches an edge of the parent. That point is used as attachment of the new part.

	JointPivot* upstreamJoint = nullptr;
	bool useUpstreamJoint = partMustGenerateJoint(newBodyPartType);
	if (useUpstreamJoint) {
		// we cannot grow this part directly onto its parent, they must be connected by a joint
		upstreamJoint = new JointPivot();
//		parent->add(upstreamJoint, angle);

		// set part to point to the joint's node, since that's where the actual part will be attached:
		parent = upstreamJoint;
		// recompute coordinates in joint's space:
		//angle = 0;
	}

	BodyPart* bp = nullptr;
	IMotor* pMotor = nullptr;
	ISensor* pSensor = nullptr;
	switch (newBodyPartType) {
	case BodyPartType::BONE:
		bp = new Bone();
		break;
	case BodyPartType::GRIPPER: {
		Gripper* gr = nullptr;//new Gripper();
		pMotor = gr;
		bp = gr;
		break;
	}
	case BodyPartType::MUSCLE: {
		// muscle must be linked to the nearest joint - or one towards which it's oriented if equidistant
		// linkage is postponed until before commit when all parts are in place (muscle may be created before joint)
		Muscle* m = nullptr;//new Muscle();
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
		EggLayer* e = nullptr;//new EggLayer();
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
		mapJointOffsets_[bp] = std::make_pair(upstreamJoint, CumulativeValue());
	}

	//parent->add(bp, angle);

	// this must happen AFTER the part is added to its parent:
	if (pMotor)
		addMotor(pMotor, bp);
	if (pSensor)
		addSensor(pSensor);

	// start a new development path from the new part:
	activeSet_.push_back(std::make_pair(bp, genomeOffset));
}*/

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

//bool Ribosome::hasNeuron(int virtualIndex, bool physical) {
//	 bool hasVirtual = mapNeurons_.find(virtualIndex) != mapNeurons_.end();
//	 if (physical)
//		 return hasVirtual && bug_->neuralNet_ && mapNeurons_[virtualIndex].index < bug_->neuralNet_->neurons.size();
//	 else
//		 return hasVirtual;
//}
//
//void Ribosome::checkAndAddNeuronMapping(int virtualIndex) {
//	if (!hasNeuron(virtualIndex, false)) {
//		int realIndex = mapNeurons_.size();
//		mapNeurons_[virtualIndex] = NeuronInfo(realIndex);
//	}
//}

void Ribosome::decodeGene(Gene const& g, BodyCell &cell, DecodeContext &ctx, bool deferNeural) {
	switch (g.type) {
	case gene_type::NO_OP:
		break;
	case gene_type::SKIP:
		break;
	case gene_type::STOP:
		break;
	case gene_type::PROTEIN:
		decodeProtein(g.data.gene_protein, cell, ctx);
		break;
	case gene_type::OFFSET:
		decodeOffset(g.data.gene_offset, cell, ctx);
		break;
	case gene_type::DIVISION_PARAM:
		decodeDivisionParam(g.data.gene_division_param, cell, ctx);
		break;
	case gene_type::PART_ATTRIBUTE:
		decodePartAttrib(g.data.gene_attribute, cell, ctx);
		break;
	case gene_type::JOINT_ATTRIBUTE:
		decodeJointAttrib(g.data.gene_joint_attrib, cell, ctx);
		break;
	case gene_type::MUSCLE_ATTRIBUTE:
		decodeMuscleAttrib(g.data.gene_muscle_attrib, cell, ctx);
		break;
	case gene_type::BODY_ATTRIBUTE:
		bodyAttribGenes_.insert(&g);
		//bug_->mapBodyAttributes_[g.data.gene_body_attribute.attribute]->changeAbs(g.data.gene_body_attribute.value);
		break;
	case gene_type::VMS_OFFSET:
		decodeVMSOffset(g.data.gene_vms_offset, cell, ctx);
		break;
	case gene_type::NEURON:
	case gene_type::SYNAPSE:
	case gene_type::NEURON_INPUT_COORD:
	case gene_type::NEURON_OUTPUT_COORD:
	case gene_type::TRANSFER_FUNC:
	case gene_type::NEURAL_BIAS:
	case gene_type::NEURAL_PARAM:
		neuralGenes_.insert(&g);
		break;
	default:
		ERROR("Unhandled gene type : " << (uint)g.type);
	}
}

/*bool Ribosome::partMustGenerateJoint(BodyPartType part_type) {
	switch (part_type) {
	case BodyPartType::BONE:
	case BodyPartType::GRIPPER:
		return true;
	default:
		return false;
	}
}*/

void Ribosome::decodeProtein(GeneProtein const& g, BodyCell &cell, DecodeContext &ctx) {
	cell.proteinValues_[g.protein - GENE_PROT_X] += g.weight;
}

void Ribosome::decodeOffset(GeneOffset const& g, BodyCell &cell, DecodeContext &ctx) {
	if (g.side >= 0)
		ctx.childOffsets[0].changeAbs(g.offset);
	if (g.side <= 0)
		ctx.childOffsets[1].changeAbs(g.offset);
}

void Ribosome::decodeDivisionParam(GeneDivisionParam const& g, BodyCell &cell, DecodeContext &ctx) {
	if (g.param > GENE_DIVISION_INVALID && g.param < GENE_DIVISION_END)
		cell.mapDivisionParams_[g.param].changeAbs(g.value);
}

void Ribosome::decodePartAttrib(GeneAttribute const& g, BodyCell &cell, DecodeContext &ctx) {
	if (g.attribute > GENE_ATTRIB_INVALID && g.attribute < GENE_ATTRIB_END)
		cell.mapAttributes_[g.attribute].changeAbs(g.value);
	if (g.attribute == GENE_ATTRIB_LOCAL_ROTATION)
		cell.updateRotation();
}

void Ribosome::decodeJointAttrib(GeneJointAttribute const& g, BodyCell &cell, DecodeContext &ctx) {
	if (g.attrib > GENE_JOINT_ATTR_INVALID && g.attrib < GENE_JOINT_ATTR_END)
		cell.mapJointAttribs_[g.attrib].changeAbs(g.value);
}

void Ribosome::decodeMuscleAttrib(GeneMuscleAttribute const& g, BodyCell &cell, DecodeContext &ctx) {
	if (g.attrib > GENE_MUSCLE_ATTR_INVALID && g.attrib < GENE_MUSCLE_ATTR_END) {
		if (g.side >= 0)
			cell.mapLeftMuscleAttribs_[g.attrib].changeAbs(g.value);
		if (g.side <= 0)
			cell.mapRightMuscleAttribs_[g.attrib].changeAbs(g.value);
	}
}

void Ribosome::decodeVMSOffset(GeneVMSOffset const& g, BodyCell &cell, DecodeContext &ctx) {
	cell.VMSOffset_.changeAbs(g.value);
}

void Ribosome::decodeSynapse(GeneSynapse const& g) {
	// the number of neurons is derived from the synapse values
//	checkAndAddNeuronMapping(g.from);
//	checkAndAddNeuronMapping(g.to);
//	uint64_t key = synKey(g.from, g.to);
//	assert(!std::isnan(g.weight.value));
//	mapSynapses_[key].weight.changeAbs(g.weight);
//	mapSynapses_[key].priority.changeAbs(g.priority);
	throw std::runtime_error("Not implemented!");
}

void Ribosome::decodeTransferFn(GeneTransferFunction const& g) {
//	if (hasNeuron(g.targetNeuron, false))
//		mapNeurons_[g.targetNeuron].transfer.changeAbs(g.functionID);
	throw std::runtime_error("Not implemented!");
}

void Ribosome::decodeNeuralBias(GeneNeuralBias const& g) {
	assert(!std::isnan(g.value.value));
//	if (hasNeuron(g.targetNeuron, false))
//		mapNeurons_[g.targetNeuron].bias.changeAbs(g.value);
	throw std::runtime_error("Not implemented!");
}

void Ribosome::decodeNeuralParam(GeneNeuralParam const& g) {
	assert(!std::isnan(g.value.value));
//	if (hasNeuron(g.targetNeuron, false))
//		mapNeurons_[g.targetNeuron].param.changeAbs(g.value);
	throw std::runtime_error("Not implemented!");
}

void Ribosome::decodeNeuronOutputCoord(GeneNeuronOutputCoord const& g) {
//	checkAndAddNeuronMapping(g.srcNeuronVirtIndex);
//	mapNeurons_[g.srcNeuronVirtIndex].outputVMSCoord.changeAbs(g.outCoord);
//	// add this neuron into the outputNeurons_ set:
//	outputNeurons_.insert(g.srcNeuronVirtIndex);
	throw std::runtime_error("Not implemented!");
}

void Ribosome::decodeNeuronInputCoord(GeneNeuronInputCoord const& g) {
//	checkAndAddNeuronMapping(g.destNeuronVirtIndex);
//	mapNeurons_[g.destNeuronVirtIndex].inputVMSCoord.changeAbs(g.inCoord);
//	// add this neuron into the inputNeurons_ set:
//	inputNeurons_.insert(g.destNeuronVirtIndex);
	throw std::runtime_error("Not implemented!");
}

//void Ribosome::createSynapse(int from, int to, SynapseInfo const& info) {
//	assertDbg(hasNeuron(from, true));	// should be there, since synapses dictate neurons
//	assertDbg(hasNeuron(to, true));
//
//	OutputSocket* pFrom = &bug_->neuralNet_->neurons[mapNeurons_[from].index]->output;
//	Neuron* pTo = bug_->neuralNet_->neurons[mapNeurons_[to].index];
//
//	InputSocket* i = new InputSocket(pTo, info.weight);
//	pTo->addInput(std::unique_ptr<InputSocket>(i), info.priority);
//	pFrom->addTarget(i);
//	throw std::runtime_error("Not implemented!");
//}

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
//			if (false) {
//				LOGLN("LinkMotorNerve: virtN[" << mapNeuronVirtIndex_[orderedOutputNeurons_[neuronIndex].first] << "] to "
//						<< mapSockMotorInfo[orderedMotorInputs_[i].first].first << "@@"
//						<< mapSockMotorInfo[orderedMotorInputs_[i].first].second
//						<< " {lineId:" << nerveLineId << "}");
//			}
			throw std::runtime_error("Not implemented!");
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
//			mapSockMotorInfo[motors_[i]->getInputSocket(j)] = std::make_pair(motors_[i]->getMotorDebugName(), j);
			throw std::runtime_error("Not implemented!");
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
//	for (int i : inputNeurons_) {
//		if (!hasNeuron(i, true))
//			continue; // this neuron doesn't actually exist because it doesn't participate in any synapses
//		Neuron* neuron = bug_->neuralNet_->neurons[mapNeurons_[i].index];
//		float vmsCoord = mapNeurons_[i].inputVMSCoord;
//		inputNeurons.push_back(std::make_pair(neuron, vmsCoord));
//	}
//	std::vector<InputOutputNerve<Neuron*>> outputNeurons;
//	for (int i : outputNeurons_) {
//		if (!hasNeuron(i, true))
//			continue; // this neuron doesn't actually exist because it doesn't participate in any synapses
//		Neuron* neuron = bug_->neuralNet_->neurons[mapNeurons_[i].index];
//		float vmsCoord = mapNeurons_[i].outputVMSCoord;
//		outputNeurons.push_back(std::make_pair(neuron, vmsCoord));
//	}
//	// sort the input/output nerves by their VMS coords, smallest to greatest:
//	sortNervesByVMSCoord(motorInputs);
//	sortNervesByVMSCoord(sensorOutputs);
//	sortNervesByVMSCoord(outputNeurons);
//	sortNervesByVMSCoord(inputNeurons);
//
//	// link nerves to motors/sensors:
//	linkMotorNerves(outputNeurons, motorInputs);
//	linkSensorNerves(inputNeurons, sensorOutputs);
//
//	motors_.clear();
//	sensors_.clear();
//	inputNeurons_.clear();
//	outputNeurons_.clear();
	throw std::runtime_error("Not implemented!");
}

//void Ribosome::commitNeurons() {
//	for (auto &n : bug_->neuralNet_->neurons)
//		n->commitInputs();
//}

//JointPivot* Ribosome::findNearestJoint(Muscle* m, int dir) {
//	assertDbg(m->getParent() && "muscle should have a parent!");
//	int nChildren = m->getParent()->getChildrenCount();
//	std::vector<BodyPart*> bp;
//	bp.reserve(nChildren);
//	for (int i=0; i<nChildren; i++)
//		bp.push_back(m->getParent()->getChild(i));
//	std::sort(bp.begin(), bp.end(), [] (BodyPart* left, BodyPart* right) -> bool {
//		return left->getAttachmentAngle() < right->getAttachmentAngle();
//	});
//
//	int mIndex = -1;
//	for (int i=0; i<nChildren; i++) {
//		if (m->getParent()->getChild(i) == m) {
//			mIndex = i;
//			break;
//		}
//	}
//	assertDbg(mIndex >= 0 && "muscle should have been found in parent!");
//	int index = mIndex;
//	do {
//		if (dir > 0)
//			index = circularNext(index, nChildren);
//		else
//			index = circularPrev(index, nChildren);
//
//		if (m->getParent()->getChild(index)->getType() == BodyPartType::JOINT)
//			return dynamic_cast<Joint*>(m->getParent()->getChild(index));
//	} while (index != mIndex);
//	return nullptr;
//}

void Ribosome::resolveMuscleLinkage() {
//	for (Muscle* m : muscles_) {
//		JointPivot* jNeg = findNearestJoint(m, -1);
//		JointPivot* jPos = findNearestJoint(m, +1);
//		if (!jNeg && !jPos)
//			continue;
//		// default to the joint on the negative side and only select the positive one if more appropriate:
//		JointPivot* targetJoint = jNeg;
//		if (jNeg != jPos) {
//			float negDelta = absAngleDiff(jNeg->getAttachmentAngle(), m->getAttachmentAngle());
//			float posDelta = absAngleDiff(jPos->getAttachmentAngle(), m->getAttachmentAngle());
//			if (posDelta < negDelta) {
//				targetJoint = jPos;
//			} else if (posDelta == negDelta) {
//				// angle differences are equal, choose the one towards which the muscle is oriented
//				if (m->getLocalRotation() > 0) {
//					targetJoint = jPos;
//				}
//			}
//		}
//		m->setJoint(targetJoint, angleDiff(m->getAttachmentAngle(), targetJoint->getAttachmentAngle()) > 0 ? -1 : +1);
//	}
//	muscles_.clear();
	throw std::runtime_error("Not implemented!");
}

bool Ribosome::geneQualifies(Gene& g, BodyCell& c) {
	BranchRestriction *r = nullptr;
	switch (g.type) {
	case gene_type::DIVISION_PARAM:
		r = &g.data.gene_division_param.restriction;
		break;
	case gene_type::JOINT_ATTRIBUTE:
		r = &g.data.gene_joint_attrib.restriction;
		break;
	case gene_type::MUSCLE_ATTRIBUTE:
		r = &g.data.gene_muscle_attrib.restriction;
		break;
	case gene_type::OFFSET:
		r = &g.data.gene_offset.restriction;
		break;
	case gene_type::PART_ATTRIBUTE:
		r = &g.data.gene_attribute.restriction;
		break;
	case gene_type::PROTEIN:
		r = &g.data.gene_protein.restriction;
		break;
	case gene_type::SKIP:
		r = &g.data.gene_skip.restriction;
		break;
	case gene_type::VMS_OFFSET:
		r = &g.data.gene_vms_offset.restriction;
		break;
	default:
		break;
	}
	if (!r)
		return true;
	// check for propagation block down to the cell
	for (uint i=0; i<c.branch_.size(); i++) {
		bool block = c.branch_[i] == 'R' ? fBool(r->levels[i].stopRight) : fBool(r->levels[i].stopLeft);
		if (block)
			return false;
	}
	// check for apply restrictions at cell's level:
	auto depth = c.branch_.size();
	auto activeLevels = r->activeLevels.value % constants::MAX_DIVISION_DEPTH;
	if (depth < activeLevels) {
		char side = depth > 0 ? c.branch_.back() : '0';
		bool block = side == 'R' ? fBool(r->levels[depth].skipRight) :
					 (side == 'L' ? fBool(r->levels[depth].skipLeft) :
							 fBool(r->levels[0].skipLeft) || fBool(r->levels[0].skipRight));
		return !block;
	}
	return true;
}

void Ribosome::drawCells(RenderContext const &ctx) {
	if (!ctx.enabledLayers.bodyDebug)
		return;
	auto tr = bug_->zygoteShell_->getWorldTransformation();
	glm::mat4 m = glm::translate(glm::vec3{tr.x, tr.y, 0.f});
	m *= glm::rotate(tr.z, glm::vec3{0.f, 0.f, 1.f});
	const float scale = 0.3f;
	m *= glm::scale(glm::vec3{scale, scale, scale});
	Shape3D::get()->setTransform(m);
	for (auto c : cells_) {
		if (!c->isActive())
			continue;
		// outline
		Shape3D::get()->drawCircleXOY(c->position_, c->radius(0), 12, {0.8f, 0.8f, 0.8f});
		// properties
		glm::vec3 cpos = vec4xyz(m * glm::vec4{c->position_, 0, 1});
		auto xc = [cpos] (Viewport* viewp) -> float {
			return viewp->project(cpos).x;
		};
		auto yc = [cpos] (Viewport* viewp) -> float {
			return viewp->project(cpos).y;
		};
		if (ctx.enabledLayers.bugDebug)
			GLText::get()->print(c->rightSide_ ? "R" : "L", {xc, yc}, 0, 22, {0, 1, 1});
		if (ctx.enabledLayers.bugDebug && c->mirror_)
			GLText::get()->print("M", ViewportCoord{xc, yc} + ViewportCoord{10, 10}, 0, 22, {0, 1, 1});
		// orientation
		glm::vec2 v2 = c->position_;
		v2.x += cosf(c->angle_) * c->radius(0);
		v2.y += sinf(c->angle_) * c->radius(0);
		Shape3D::get()->drawLine({c->position_, 0}, {v2, 0}, {1, 1, 0, 0.7f});
		// division axis
		float angle = limitAngle(c->mapDivisionParams_[GENE_DIVISION_ANGLE].get(), PI);
		v2 = c->position_;
		v2.x += cosf(c->wangle(angle)) * c->radius(0) * 1.2f;
		v2.y += sinf(c->wangle(angle)) * c->radius(0) * 1.2f;
		glm::vec2 v1 = c->position_ - (v2 - c->position_) * 0.7f;
		Shape3D::get()->drawLine({v1, 0}, {v2, 0}, {1, 0, 0, 0.5f});

		// bonds
		for (auto l : c->neighbours_) {
			if (l.offset == 0) {
				// weld joint
				glm::vec2 v1 = c->position_;
				glm::vec2 v2 = v1;
				v2.x += cosf(c->wangle(l.angle)) * c->radius(0);
				v2.y += sinf(c->wangle(l.angle)) * c->radius(0);
				v1 += (v2-v1) * 0.9f;
				Shape3D::get()->drawLine({v1, 0}, {v2, 0}, {0, 1, 1});
			} else {
				// pivot joint
				float jr = l.offset/2;
				glm::vec2 jc = c->position_;
				jc.x += cosf(c->wangle(l.angle)) * (c->radius(0) + jr);
				jc.y += sinf(c->wangle(l.angle)) * (c->radius(0) + jr);
				Shape3D::get()->drawCircleXOY(jc, jr, 8, {1.f, 0.2f, 0.1f});
			}
		}
	}
	Shape3D::get()->resetTransform();
}
