#include "Ribosome.h"

#include "../entities/Bug/Bug.h"
#include "../body-parts/BodyConst.h"
#include "../body-parts/BodyCell.h"
#include "../body-parts/BodyPart.h"
#include "../body-parts/Bone.h"
#include "../body-parts/Gripper.h"
#include "../body-parts/ZygoteShell.h"
#include "../body-parts/Muscle.h"
#include "../body-parts/Mouth.h"
#include "../body-parts/EggLayer.h"
#include "../body-parts/sensors/Nose.h"
#include "../body-parts/sensors/Eye.h"
#include "../body-parts/sensors/Compass.h"
#include "../body-parts/JointPivot.h"
#include "../body-parts/JointWeld.h"
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
#include <boglfw/renderOpenGL/GLText.h>

#ifdef DEBUG
#include <boglfw/World.h>
#endif

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
	{ BodyPartType::SENSOR_COMPASS, 	Compass::getDensity },
	{ BodyPartType::SENSOR_PROXIMITY,	Nose::getDensity },
	{ BodyPartType::SENSOR_SIGHT, 		Eye::getDensity },
};

static std::map<BodyPartType, float (*)(BodyCell const& cell, float angle)> mapRadiusFunctions {
	{ BodyPartType::BONE,				Bone::getRadius },
	{ BodyPartType::EGGLAYER,			EggLayer::getRadius },
	{ BodyPartType::FAT, 				FatCell::getRadius },
	{ BodyPartType::GRIPPER, 			Gripper::getRadius },
	{ BodyPartType::MOUTH, 				Mouth::getRadius },
	{ BodyPartType::SENSOR_COMPASS, 	Compass::getRadius },
	{ BodyPartType::SENSOR_PROXIMITY,	Nose::getRadius },
	{ BodyPartType::SENSOR_SIGHT, 		Eye::getRadius },
};

Ribosome::Ribosome(Bug* bug)
	: bug_{bug}
{
	// there are no default body parts; they either get created by the genes, or the embryo
	// is discarded at the end of development if it lacks critical parts such as mouth or egg-layer

	float initialSize = bug->zygoteShell_->size();
	BodyCell* initialCell = new BodyCell(initialSize, glm::vec2(0, 0), 0, false, false);
	cells_.push_back(initialCell);

	bug_->neuralNet_ = new NeuralNet();

	// start decoding with root cell at offset 0 in the genome:
	activeSet_.push_back({initialCell, 0});
}

Ribosome::~Ribosome() {
	cleanUp();
}

void Ribosome::cleanUp() {
	for (auto c : cells_)
		delete c;
	cells_.clear();
	activeSet_.clear();
	cellContext_.clear();
	bodyAttribGenes_.clear();
	motors_.clear();
//	mapInputNerves_.clear();
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

bool Ribosome::step() {
	LOGPREFIX("Ribosome");
	if (activeSet_.empty()) {
		// finished decoding & division
		postDecodeAndFinalization();
#ifdef DEBUG
		World::getInstance().triggerEvent("pauseRequested", 1);
		World::getInstance().triggerEvent("slowMoRequested", 1);
#endif
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
			if (cell->mapDivisionParams_[GENE_DIVISION_AFFINITY] > 0.f
					&& cell->branch_.size() < constants::MAX_DIVISION_DEPTH
					&& cell->size() >= BodyConst::CellDivisionSizeThresh) {
				// check if division will create pivot joint, and if so, we need to subtract the mass required to make the joint and muscles
				float cellMass = cell->density() * cell->size();
				float jointMass = cellMass * cell->mapJointAttribs_[GENE_JOINT_ATTR_MASS_RATIO].clamp(
						BodyConst::MinJointMassRatio, BodyConst::MaxJointMassRatio);
				// make sure joint mass doesn't exceed either child cell mass
				float divisionRatio = cell->mapDivisionParams_[GENE_DIVISION_RATIO].clamp(
											BodyConst::minDivisionRatio,
											1.f / BodyConst::minDivisionRatio);
				if (divisionRatio > 0.5)
					divisionRatio = 1 - divisionRatio;	// use the smaller child cell as comparison
				if (jointMass > cellMass * divisionRatio)
					jointMass = cellMass * divisionRatio;

				float muscleMass = 0;
				if (cell->mapJointAttribs_[GENE_JOINT_ATTR_TYPE] > 0.f) {
					muscleMass += cell->muscleMassLeft_ = (cellMass-jointMass) * cell->mapLeftMuscleAttribs_[GENE_MUSCLE_ATTR_MASS_RATIO].clamp(
							BodyConst::MinMuscleMassRatio, BodyConst::MaxMuscleMassRatio);
					muscleMass += cell->muscleMassRight_ = (cellMass-jointMass) * cell->mapRightMuscleAttribs_[GENE_MUSCLE_ATTR_MASS_RATIO].clamp(
							BodyConst::MinMuscleMassRatio, BodyConst::MaxMuscleMassRatio);
					cell->setJointSize(jointMass / cell->mapJointAttribs_[GENE_JOINT_ATTR_DENSITY].clamp(
							BodyConst::MinBodyPartDensity, BodyConst::MaxBodyPartDensity));
				}
				cell->size_ *= (cellMass - jointMass - muscleMass) / cellMass;
				cell->jointMass_ = jointMass;
				// divide
//#ifdef DEBUG
//				if (cell->matchBranch("LLRR")) {
//					LOGLN("divide #4e; offsets: " << activeSet_[i].second.childOffsets[0] << " : " << activeSet_[i].second.childOffsets[1]);
//				}
//#endif
				auto pair = cell->divide();
				cells_.push_back(pair.first);
				cells_.push_back(pair.second);
				DecodeContext leftCtx(activeSet_[i].second.startGenomePos + activeSet_[i].second.childOffsets[0]);
				// inherit vms offset and offset genes set
				leftCtx.parentVmsOffset = activeSet_[i].second.parentVmsOffset + activeSet_[i].second.vmsOffset;
				leftCtx.vmsOffsetGenes = activeSet_[i].second.vmsOffsetGenes;
				DecodeContext rightCtx(activeSet_[i].second.startGenomePos + activeSet_[i].second.childOffsets[1]);
				// inherit vms offset and offset genes set
				rightCtx.parentVmsOffset = activeSet_[i].second.parentVmsOffset + activeSet_[i].second.vmsOffset;
				rightCtx.vmsOffsetGenes = activeSet_[i].second.vmsOffsetGenes;
				// start decoding the children cells
				activeSet_.push_back({pair.first, leftCtx});
				activeSet_.push_back({pair.second, rightCtx});
			} else {
				// this cell will specialize, we need to keep its data
				cell->vmsOffset_ = activeSet_[i].second.vmsOffset.get() + activeSet_[i].second.parentVmsOffset;
				cellContext_[cell] = std::move(activeSet_[i].second);
			}
			// remove this branch:
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
		activeSet_[i].second.crtGenomePos += abs(skipCount); // only skip forward, never back to avoid infinite loop
	}
	return true;
}

void Ribosome::postDecodeAndFinalization() {
	// check if critical body parts exist (at least a mouth and egg-layer)
	bool hasMouth = false, hasEggLayer = false;
	specializeCells(hasMouth, hasEggLayer);

	if (!researchMode_ && (!hasMouth || !hasEggLayer)) {
		// here mark the embryo as non viable and return
		bug_->isViable_ = false;
		cleanUp();
		return;
	}

	// decode body attrib genes:
	for (auto g : bodyAttribGenes_)
		bug_->mapBodyAttributes_[g->data.gene_body_attribute.attribute]->changeAbs(g->data.gene_body_attribute.value);

	// decode the neural network:
	decodeDeferredGenes();
	// link nerves to sensors and motors:
	resolveMotorLinkage();
	// commit neuron properties:
	commitNeurons();

	// clean up:
	cleanUp();
}

BodyPartType Ribosome::specializationType(BodyCell const& c) const {
	if (c.proteinValues_.x * c.proteinValues_.y * c.proteinValues_.z * c.proteinValues_.w == 0.f)	// incomplete specialization -> FAT
		return BodyPartType::FAT;
	return proteinHyperspace
			[c.proteinValues_.w > 0 ? 1 : 0]
			[c.proteinValues_.z > 0 ? 1 : 0]
			[c.proteinValues_.y > 0 ? 1 : 0]
			[c.proteinValues_.x > 0 ? 1 : 0];
}

void Ribosome::specializeCells(bool &hasMouth, bool &hasEggLayer) {
	// first run: update cells' density, then set radiusFn in all cells, then call fixOverlap on each of them
	std::set<BodyCell*> activeCells;
	for (auto c : cells_) {
		if (!c->isActive())
			continue;
		// update cell's density:
		updateCellDensity(*c);
		// adjust the cell's shape:
		c->radiusFn = mapRadiusFunctions[specializationType(*c)];
		assert(c->radiusFn != nullptr);
		activeCells.insert(c);

		switch (specializationType(*c)) {
		case BodyPartType::MOUTH:
			hasMouth = true;
			break;
		case BodyPartType::EGGLAYER:
			hasEggLayer = true;
			break;
		default:
			break;
		}
	}

	if (!researchMode_ && (!hasMouth || !hasEggLayer)) {
		return;
	}

	// fix all cell's positioning (affected by changing shape and size above)
	std::set<Cell*> cellSet { activeCells.begin(), activeCells.end() };
	BodyCell::fixOverlap(cellSet, !researchMode_);
	for (auto c : activeCells)
		c->updateBonds();

	auto tr = bug_->zygoteShell_->getWorldTransformation();
	glm::mat4 m = glm::translate(glm::vec3{tr.x, tr.y, 0.f});
	m *= glm::rotate(tr.z, glm::vec3{0.f, 0.f, 1.f});

	// second run: instantiate body parts:
	for (auto c : activeCells) {
		c->transform(m, tr.z);
		BodyPart* bp = nullptr;
		IMotor* pMotor = nullptr;
		ISensor* pSensor = nullptr;
		switch (specializationType(*c)) {
		case BodyPartType::MOUTH:
			bp = new Mouth(bug_->context_, *c);
			break;
		case BodyPartType::EGGLAYER: {
			auto e = new EggLayer(bug_->context_, *c);
			bp = e;
			bug_->eggLayers_.push_back(e);
		} break;
		case BodyPartType::BONE:
			bp = new Bone(bug_->context_, *c);
			break;
		case BodyPartType::FAT:
			bp = new FatCell(bug_->context_, *c);
			break;
		case BodyPartType::GRIPPER: {
			auto g = new Gripper(bug_->context_, *c);
			bp = g;
			pMotor = g;
		} break;
		case BodyPartType::SENSOR_COMPASS: {
			auto comp = new Compass(bug_->context_, *c);
			bp = comp;
			pSensor = comp;
		} break;
		case BodyPartType::SENSOR_PROXIMITY: {
			auto n = new Nose(bug_->context_, *c);
			bp = n;
			pSensor = n;
		} break;
		case BodyPartType::SENSOR_SIGHT: {
			auto e = new Eye(bug_->context_, *c);
			pSensor = e;
			bp = e;
		} break;
		default:
			throw std::runtime_error("invalid specialization type!");
		};

		if (bp) {
			bug_->bodyParts_.push_back(bp);
			cellToPart_[c] = bp;
		}
		if (pMotor)
			motors_[pMotor] = &cellContext_[c];
		if (pSensor)
			cellContext_[c].sensors_.push_back(pSensor);
	}
	// TODO Auto-generate body-part-sensors in joints & grippers and other parts that may have useful info

	// 3rd run: create joints and muscles
	std::set<std::pair<Cell*, Cell*>> joints;
	for (auto c : activeCells) {
		for (auto &n : c->neighbours_) {
			Cell *left = c, *right = n.other;
			if (n.isRightSide) {
				xchg(left, right);
			}
			if (joints.insert({left, right}).second) {
				// only create each joint once - it will appear in both sides of the split
				BodyPart* bpLeft = cellToPart_[left];
				BodyPart* bpRight = cellToPart_[right];
				assert(bpLeft && bpRight && "bodyparts for each side of the joint must exist!");
				BodyCell* jointCell = static_cast<BodyCell*>(n.jointParent);
				assert(jointCell && "joint cell must not be null!");
				bool pivotJoint = jointCell->mapJointAttribs_[GENE_JOINT_ATTR_TYPE] > 0.f;
				Joint* j = nullptr;
				if (pivotJoint) {
					auto pj = new JointPivot(bug_->context_, *jointCell, bpLeft, bpRight);
					j = pj;
					Muscle *ml = new Muscle(bug_->context_, *jointCell, false);
					ml->setJoint(pj);
					Muscle *mr = new Muscle(bug_->context_, *jointCell, true);
					mr->setJoint(pj);
					bug_->bodyParts_.push_back(ml);
					bug_->bodyParts_.push_back(mr);
					motors_[ml] = &cellContext_[static_cast<BodyCell*>(left)]; //the muscle motor will link to neurons in the left cell
					motors_[mr] = &cellContext_[static_cast<BodyCell*>(left)];


					// muscles are linked to the left cell
					bpLeft->addNeighbor(ml);
					ml->addNeighbor(bpLeft);
					bpLeft->addNeighbor(mr);
					mr->addNeighbor(bpLeft);
				} else {
					auto wj = new JointWeld(bug_->context_, *jointCell, bpLeft, bpRight);
					j = wj;
				}
				cellToPart_[jointCell] = j;
				bug_->bodyParts_.push_back(j);
				j->onJointBreak.add(std::bind(&Bug::onJointBreak, bug_, std::placeholders::_1));

				bpLeft->addNeighbor(j);
				j->addNeighbor(bpLeft);
				bpRight->addNeighbor(j);
				j->addNeighbor(bpRight);
			}
		}
	}
}

// call this before instantiating the body part in order to update to correct density and size
void Ribosome::updateCellDensity(BodyCell &cell) {
	auto fn = mapDensityFunctions[specializationType(cell)];
	assert(fn != nullptr);
	auto oldDensity = cell.density_;
	cell.density_ = fn(cell);
	// must adjust cell size to conserve mass
	cell.size_ *= oldDensity / cell.density_;
}

//void Ribosome::addMotor(IMotor* motor, BodyPart* part) {
//	motors_.push_back(motor);
//	for (unsigned i=0; i<motor->getInputCount(); i++) {
//		int lineId = nMotorLines_++;
//		part->addMotorLine(lineId);
//	}
//}

void Ribosome::createNeurons(BodyCell& cell, DecodeContext &ctx) {
	// create the default neuron:
	float vmsOffset = cell.vmsOffset();
	BodyPart* part = cellToPart_[&cell];
	Neuron* dn = new Neuron();
	bug_->neuralNet_->neurons.push_back(dn);
	bug_->bodyPartNeurons_[part].push_back(dn);
	ctx.vmsNeurons_.push_back({NeuronInfo(dn), vmsOffset});
	// create the neurons from genes:
	for (auto g : ctx.neuralGenes) {
		if (g->type == gene_type::NEURON) {
			// instantiate new neuron in the current cell
			Neuron* n = new Neuron();
			bug_->neuralNet_->neurons.push_back(n);
			float geneVMSValue = clamp(g->data.gene_neuron.neuronLocation.value, -BodyConst::MaxVMSCoordinateValue, BodyConst::MaxVMSCoordinateValue);
			geneVMSValue *= cell.vmsDirection();
			ctx.vmsNeurons_.push_back({NeuronInfo(n), vmsOffset + geneVMSValue});
			bug_->bodyPartNeurons_[part].push_back(n);
		}
	}
}

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
		break;
	case gene_type::VMS_OFFSET:
		decodeVMSOffset(g.data.gene_vms_offset, cell, ctx);
		break;
	case gene_type::NEURON:
	case gene_type::SYNAPSE:
	case gene_type::TRANSFER_FUNC:
	case gene_type::NEURAL_BIAS:
	case gene_type::NEURAL_PARAM:
	case gene_type::TIME_SYNAPSE:
		ctx.neuralGenes.push_back(&g);
		break;
	default:
		ERROR("Unhandled gene type : " << (unsigned)g.type);
	}
}

void Ribosome::decodeDeferredGenes() {
	// step 1 : instantiate neurons in all cells
	for (auto &it : cellContext_) {
		BodyCell* cell = it.first;
		DecodeContext& ctx = it.second;
		createNeurons(*cell, ctx);
		// sort neurons by their vms coord
		sortEntriesByVMSCoord(ctx.vmsNeurons_);
	}

	// step 2: decode neuron properties and synapses
	for (auto &it : cellContext_) {
		BodyCell* cell = it.first;
		DecodeContext& ctx = it.second;
		float cellOffs = cell->vmsOffset();

		/* build a list of output sockets from these objects:
		 * 		- neurons within the cell
		 * 		- sensors within the cell
		 * 		- neurons from immediate neighbor cells
		*/
		std::vector<VMSEntry<std::pair<OutputSocket*, BodyCell*>>> outputSockets;
		buildOutputSocketsList(cell, outputSockets);
		std::map<std::pair<OutputSocket*, Neuron*>, SynapseInfo> mapSynapses;
		// decode the deferred neural genes (neuron properties):
		for (auto g : ctx.neuralGenes) {
			if (g->type == gene_type::NEURON)
				continue;	// neurons have already been created
			decodeNeuralGene(*g, cellOffs, cell->vmsDirection(), outputSockets, ctx.vmsNeurons_, mapSynapses);
		}
		// create the synapses:
		for (auto &p : mapSynapses) {
			OutputSocket* outSock = p.first.first;	// source
			Neuron* neuron = p.first.second;		// sink
			SynapseInfo& sInfo = p.second;
			InputSocket* i = new InputSocket(neuron, sInfo.weight);
			neuron->addInput(std::unique_ptr<InputSocket>(i), sInfo.priority);
			outSock->addTarget(i);
			bug_->jointSynapses_[p.second.ownerJoint].push_back({outSock, i});
		}
	}
}

void Ribosome::buildOutputSocketsList(BodyCell* cell, std::vector<VMSEntry<std::pair<OutputSocket*, BodyCell*>>> &out) {
	// add cell's neurons' output sockets
	for (auto &vn : cellContext_[cell].vmsNeurons_) {
		float vmsCoord = vn.second;
		// neurons' vms locations is already affected by cell's vmsOffset
		OutputSocket* sock = &vn.first.neuron->output;
		out.push_back({{sock, nullptr}, vmsCoord});
	}
	// add cell's sensors' output sockets
	for (auto s : cellContext_[cell].sensors_) {
		for (unsigned i=0; i<s->getOutputCount(); i++) {
			// sensors' vms coordinates aleardy contain the cell's vmsOffset
			out.push_back({{s->getOutputSocket(i), nullptr}, s->getOutputVMSCoord(i)});
		}
	}
	// add neighbour cells' neurons' output sockets
	for (auto ncl : cell->neighbours_) {
		BodyCell* jointCell = static_cast<BodyCell*>(ncl.jointParent);	// synapse will be associated with this joint cell
		for (auto &vn : cellContext_[static_cast<BodyCell*>(ncl.other)].vmsNeurons_) {
			float vmsCoord = vn.second;
			OutputSocket* sock = &vn.first.neuron->output;
			out.push_back({{sock, jointCell}, vmsCoord});
		}
	}
	// sort all output sockets by vms coord
	sortEntriesByVMSCoord(out);
}

template <typename T>
void Ribosome::sortEntriesByVMSCoord(std::vector<VMSEntry<T>> &nerves) {
	std::sort(nerves.begin(), nerves.end(), [] (VMSEntry<T> const& left, VMSEntry<T> const& right) -> bool {
		return left.second < right.second;
	});
}


void Ribosome::decodeNeuralGene(Gene const& g, float vmsOffset, float vmsDirection,
		std::vector<VMSEntry<std::pair<OutputSocket*, BodyCell*>>> &outSockets,
		std::vector<VMSEntry<NeuronInfo>> &vmsNeurons,
		std::map<std::pair<OutputSocket*, Neuron*>, SynapseInfo> &mapSynapses) {
	switch (g.type) {
		case gene_type::SYNAPSE:
			decodeSynapse(g.data.gene_synapse, vmsOffset, vmsDirection, outSockets, vmsNeurons, mapSynapses);
			break;
		case gene_type::TIME_SYNAPSE:
			decodeTimeSynapse(g.data.gene_time_synapse, vmsOffset, vmsDirection, vmsNeurons, mapSynapses);
			break;
		case gene_type::TRANSFER_FUNC:
			decodeTransferFn(g.data.gene_transfer_function, vmsOffset, vmsDirection, vmsNeurons);
			break;
		case gene_type::NEURAL_BIAS:
			decodeNeuralBias(g.data.gene_neural_constant, vmsOffset, vmsDirection, vmsNeurons);
			break;
		case gene_type::NEURAL_PARAM:
			decodeNeuralParam(g.data.gene_neural_param, vmsOffset, vmsDirection, vmsNeurons);
			break;
		default:
			assert(!!!"Invalid neural gene type!");
			return;
	}
}

void Ribosome::decodeProtein(GeneProtein const& g, BodyCell &cell, DecodeContext &ctx) {
	cell.proteinValues_[g.protein - GENE_PROT_X] += g.weight;

//#ifdef DEBUG
//	auto pname = [] (gene_protein_type p) {
//		switch (p) {
//		case GENE_PROT_X:
//			return "X";
//		case GENE_PROT_Y:
//			return "Y";
//		case GENE_PROT_Z:
//			return "Z";
//		case GENE_PROT_W:
//			return "W";
//		default:
//			return "0";
//		}
//	};
//	if (cell.matchBranch("LLRRR")) {
//		LOGLN("new protein: " << pname(g.protein) << (g.weight > 0 ? '+' : '-') << "  " << g.weight
//				<< "\tat offset " << ctx.crtGenomePos-1);
//		LOGLN("position: "
//				<< (cell.proteinValues_[0] > 0 ? "+" : "-") << " "
//				<< (cell.proteinValues_[1] > 0 ? "+" : "-") << " "
//				<< (cell.proteinValues_[2] > 0 ? "+" : "-") << " "
//				<< (cell.proteinValues_[3] > 0 ? "+" : "-")
//				<< "\ttype:" << specializationType(cell));
//	}
//#endif
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
	float multiplier = 1.f;
	if (g.attribute >= GENE_ATTRIB_VMS_COORD1 && g.attribute <= GENE_ATTRIB_VMS_COORD5)
		multiplier = cell.vmsDirection();
	if (g.attribute > GENE_ATTRIB_INVALID && g.attribute < GENE_ATTRIB_END)
		cell.mapAttributes_[g.attribute].changeAbs(g.value * multiplier);
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
	if (ctx.vmsOffsetGenes.insert(&g).second)
		ctx.vmsOffset.changeAbs(g.value * cell.vmsDirection());
}

void Ribosome::decodeSynapse(GeneSynapse const& g, float vmsOffset, float vmsDirection,
		std::vector<VMSEntry<std::pair<OutputSocket*, BodyCell*>>> &outSockets,
		std::vector<VMSEntry<NeuronInfo>> &vmsNeurons,
		std::map<std::pair<OutputSocket*, Neuron*>, SynapseInfo> &mapSynapses)
{
	float vmsFrom = vmsOffset + vmsDirection * clamp(g.srcLocation.value, -BodyConst::MaxVMSCoordinateValue, BodyConst::MaxVMSCoordinateValue);
	float vmsTo = vmsOffset + vmsDirection * clamp(g.destLocation.value, -BodyConst::MaxVMSCoordinateValue, BodyConst::MaxVMSCoordinateValue);
	auto iTo = getVMSNearestObjectIndex(vmsNeurons, vmsTo, [](auto){return true;});
	auto iFrom = getVMSNearestObjectIndex(outSockets, vmsFrom, [&] (auto sock) {
		return &vmsNeurons[iTo].first.neuron->output != sock.first;	// don't create synapse between same neuron's output and input
	});

	if (iFrom == -1 || iTo == -1) {
		ERROR("Synapse to/from non-existent sensor/neuron!!");
		return;
	}

	OutputSocket *from = outSockets[iFrom].first.first;
	NeuronInfo& to = vmsNeurons[iTo].first;
	Joint* j = static_cast<Joint*>(cellToPart_[outSockets[iFrom].first.second]);
	updateSynapseInfo(from, to, j, mapSynapses, g.priority, g.weight);
}

void Ribosome::decodeTimeSynapse(GeneTimeSynapse const& g, float vmsOffset, float vmsDirection,
		std::vector<VMSEntry<NeuronInfo>> &vmsNeurons,
		std::map<std::pair<OutputSocket*, Neuron*>, SynapseInfo> &mapSynapses)
{
	float vmsTo = vmsOffset + vmsDirection * clamp(g.targetLocation.value, -BodyConst::MaxVMSCoordinateValue, BodyConst::MaxVMSCoordinateValue);
	auto iTo = getVMSNearestObjectIndex(vmsNeurons, vmsTo, [](auto){return true;});

	if (iTo == -1) {
		ERROR("Time Synapse to non-existent neuron!!");
		return;
	}

	OutputSocket *from = &bug_->lifetimeOutput_;
	NeuronInfo& to = vmsNeurons[iTo].first;
	updateSynapseInfo(from, to, nullptr, mapSynapses, 0, g.weight);
}

void Ribosome::updateSynapseInfo(OutputSocket *from, NeuronInfo &to, Joint* synJoint,
		std::map<std::pair<OutputSocket*, Neuron*>, SynapseInfo> &mapSynapses,
		float priority, float weight)
{
	// if the same synapse (from the same sensor/neuron to the same neuron) has already been created
	// only update its properties (weight etc) instead of creating a new one
	auto synapseKey = std::make_pair(from, to.neuron);
	auto &sInfo = mapSynapses[synapseKey]; // this works because the value is automatically created in the map the first time
	sInfo.priority.changeAbs(priority);
	sInfo.weight.changeAbs(weight);
	sInfo.ownerJoint = synJoint;
}

void Ribosome::decodeTransferFn(GeneTransferFunction const& g, float vmsOffset, float vmsDirection,
		std::vector<VMSEntry<NeuronInfo>> &vmsNeurons)
{
	float vmsTarget = vmsOffset + vmsDirection * clamp(g.neuronLocation.value, -BodyConst::MaxVMSCoordinateValue, BodyConst::MaxVMSCoordinateValue);
	auto iN = getVMSNearestObjectIndex(vmsNeurons, vmsTarget, [](auto){return true;});
	if (iN >= 0)
		vmsNeurons[iN].first.transfer.changeAbs(g.functionID);
}

void Ribosome::decodeNeuralBias(GeneNeuralBias const& g, float vmsOffset, float vmsDirection,
		std::vector<VMSEntry<NeuronInfo>> &vmsNeurons)
{
	assert(!std::isnan(g.value.value));
	float vmsTarget = vmsOffset + vmsDirection * clamp(g.neuronLocation.value, -BodyConst::MaxVMSCoordinateValue, BodyConst::MaxVMSCoordinateValue);
	auto iN = getVMSNearestObjectIndex(vmsNeurons, vmsTarget, [](auto){return true;});
	if (iN >= 0)
		vmsNeurons[iN].first.bias.changeAbs(g.value);
}

void Ribosome::decodeNeuralParam(GeneNeuralParam const& g, float vmsOffset, float vmsDirection,
		std::vector<VMSEntry<NeuronInfo>> &vmsNeurons)
{
	assert(!std::isnan(g.value.value));
	float vmsTarget = vmsOffset + vmsDirection * clamp(g.neuronLocation.value, -BodyConst::MaxVMSCoordinateValue, BodyConst::MaxVMSCoordinateValue);
	auto iN = getVMSNearestObjectIndex(vmsNeurons, vmsTarget, [](auto){return true;});
	if (iN >= 0)
		vmsNeurons[iN].first.param.changeAbs(g.value);
}

// returns -1 if none found
template <typename T, class PredT>
int Ribosome::getVMSNearestObjectIndex(std::vector<std::pair<T, float>> const& nerves, float matchCoord,
		PredT validatePred, int start, int end) {
	if (nerves.size() == 0)
		return -1;
	if (end == -1)
		end = nerves.size() - 1;
	// binary-search the nearest object that satisfies validatePred:
	assert(start <= end);
	if (start == end) {
		if (validatePred(nerves[start].first))
			return start;
		else
			return -1;
	}
	unsigned pivot = (end-start) / 2 + start;
	if (matchCoord == nerves[pivot].second && validatePred(nerves[pivot].first))
		return pivot;	// exact match!
	// if we got here, there's no perfect match
	int iBegin, iEnd, iDir;
	if (matchCoord > nerves[pivot].second) { // look into the big interval
		iBegin = pivot + 1;
		iEnd = end;
		iDir = -1;
	} else {
		// look into the small interval
		iBegin = start;
		iEnd = pivot;
		iDir = +1;
	}
	int intervalMatch = getVMSNearestObjectIndex(nerves, matchCoord, validatePred, iBegin, iEnd);
	float intervalDelta = 1e20;
	if (intervalMatch != -1)
		intervalDelta = abs(matchCoord - nerves[intervalMatch].second);
	// go back from the pivot one by one and get the closest match and compare it to the intervalMatch
	int otherSearch = pivot + (iDir > 0 ? iDir : 0);
	while (otherSearch >= start && otherSearch <= end && !validatePred(nerves[otherSearch].first))
		otherSearch += iDir;
	if (otherSearch >= start && otherSearch <= end) {
		// we found something in the other interval as well, let's see which is closer
		float otherDelta = abs(matchCoord - nerves[otherSearch].second);
		if (otherDelta < intervalDelta)
			return otherSearch;
		else
			return intervalMatch;
	} else if (intervalMatch != -1)
		return intervalMatch;
	else
		return -1;
}

void Ribosome::linkMotorNerves(std::vector<VMSEntry<NeuronInfo>> const& neurons, std::vector<VMSEntry<InputSocket*>> const& orderedMotorInputs_) {
	// motors are matched 1:1 with the nearest neuron outputs from the neural network, in the direction from motor nerve to output nerve.
	for (unsigned i = 0; i < orderedMotorInputs_.size(); i++) {
		float motorCoord = orderedMotorInputs_[i].second;
		int neuronIndex = getVMSNearestObjectIndex(neurons, motorCoord, [](auto){return true;});
		if (neuronIndex >= 0) {
			// link this motor to this neuron
			neurons[neuronIndex].first.neuron->output.addTarget(orderedMotorInputs_[i].first);
		}
	}
}

void Ribosome::resolveMotorLinkage() {
	for (auto &p : motors_) {
		// build the motor input nerves vector:
		std::vector<VMSEntry<InputSocket*>> motorInputs;
		for (unsigned j=0; j<p.first->getInputCount(); j++) {
			motorInputs.push_back({p.first->getInputSocket(j), p.first->getInputVMSCoord(j)});
		}
		// sort the input/output nerves by their VMS coords, smallest to greatest:
		sortEntriesByVMSCoord(motorInputs);
		// link motors to neuron outputs from within the motor's cell:
		// (at this point p.second->vmsNeurons_ is already sorted by vms coord)
		linkMotorNerves(p.second->vmsNeurons_, motorInputs);
	}
}

void Ribosome::commitNeurons() {
	for (auto &p : cellContext_) {
		// apply all neuron properties
		for (auto &vn : p.second.vmsNeurons_) {
			NeuronInfo &n = vn.first;
			if (n.transfer.hasValue()) {
				int funcIndex = clamp((int)n.transfer.get(),
						(int)transferFuncNames::FN_ONE,
						(int)transferFuncNames::FN_MAXCOUNT-1);
				n.neuron->setTranferFunction((transferFuncNames)funcIndex);
			}
			if (n.bias.hasValue())
				n.neuron->inputBias = n.bias;
			if (n.param.hasValue())
				n.neuron->neuralParam = n.param;
		}
	}
	// apply input priorities:
	for (auto &n : bug_->neuralNet_->neurons)
		n->commitInputs();
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
	for (unsigned i=0; i<c.branch_.size(); i++) {
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

void Ribosome::drawCells(Viewport* vp) {
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
		if (false) {
			GLText::get()->print(c->rightSide_ ? "R" : "L", {xc, yc}, 0, 22, {0, 1, 1});
			if (c->mirror_)
				GLText::get()->print("M", ViewportCoord{xc, yc} + ViewportCoord{10, 10}, 0, 22, {0, 1, 1});
		}
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
		if (true) {
			for (auto l : c->neighbours_) {
				if (l.offset == 0) {
					// weld joint
					glm::vec2 v1 = c->position_;
					glm::vec2 v2 = v1;
					v2.x += cosf(c->wangle(l.angle)) * c->radius(0);
					v2.y += sinf(c->wangle(l.angle)) * c->radius(0);
					v1 += (v2-v1) * 0.9f;
					glm::vec3 color = l.isRightSide ? glm::vec3{0, 0, 1} : glm::vec3{0, 1, 1};
					Shape3D::get()->drawLine({v1, 0}, {v2, 0}, color);
				} else if (!l.isRightSide) {
					// pivot joint
					float jr = l.offset/2;
					glm::vec2 jc = c->position_;
					jc.x += cosf(c->wangle(l.angle)) * (c->radius(0) + jr);
					jc.y += sinf(c->wangle(l.angle)) * (c->radius(0) + jr);
					Shape3D::get()->drawCircleXOY(jc, jr, 8, {1.f, 0.2f, 0.1f});
				}
			}
		}
	}
	Shape3D::get()->resetTransform();
}
