/*
 *	a ribosome decodes a sequence of chromosomes and builds a fully-functional neural network
 */

#ifndef __ribosome_h__
#define __ribosome_h__

#include "Genome.h"
#include "Gene.h"
#include "CumulativeValue.h"
#include "../body-parts/BodyPart.h"

#include <glm/vec4.hpp>
#include <vector>
#include <set>
#include <map>

class Bug;
class BodyPart;
class Gene;
class Neuron;
class InputSocket;
class OutputSocket;
class IMotor;
class ISensor;
class Muscle;
class JointPivot;

/*struct NeuronInfo {
	int index;
	CumulativeValue transfer;
	CumulativeValue bias;
	CumulativeValue param;
	CumulativeValue inputVMSCoord;
	CumulativeValue outputVMSCoord;
	/*NeuronInfo(int index, float transfer, float constant)
		: index(index), transfer(transfer), bias(constant) {
	}* /
	explicit NeuronInfo(int index)
		: index(index) {
	}
	NeuronInfo(NeuronInfo const& other) = default;
	NeuronInfo() : index(-1) {
	}
};*/

/*struct SynapseInfo {
	CumulativeValue weight;
	CumulativeValue priority;
};*/

struct DecodeContext {
	unsigned startGenomePos; // initial genome offset for this cell (children are relative to this one)
	unsigned crtGenomePos; // current READ position in genome for this cell
	CumulativeValue childOffsets[2]; // holds relative genome offsets for each future child cell (0 is left, 1 right)
	CumulativeValue vmsOffset;
	float parentVmsOffset = 0;
	std::set<const GeneVMSOffset*> vmsOffsetGenes;	// all vms offset genes are added to this set and passed down to the cell's children
											// if the same vms offset gene is encountered by the child cell it will be ignored
											// thus the vms offset gene can affect a cell only once (it is inherited by the children)
	std::vector<const Gene*> neuralGenes;

	DecodeContext(int initialOffs)
		: startGenomePos(initialOffs), crtGenomePos(initialOffs) {
	}
};

template<typename T>
using VMSEntry = std::pair<T, float>;	// first (T) is the object (neuron, input/output socket), second is the VMS coordinate

/**
 * decodes the entity's genome and builds it step by step. When finished the entity will have its final
 * shape and preprogrammed functionality, but will be very small in size.
 */
class Ribosome {
public:
	Ribosome(Bug* the_bug);
	~Ribosome();

	void addDefaultSensor(ISensor* sensor) {
		addSensor(sensor);
	}

	/**
	 * develops the entity one more step. Returns true as long as the process is not finished.
	 */
	bool step();

	void drawCells(Viewport* vp);

private:
	Bug* bug_;
	std::vector<BodyCell*> cells_;
	std::vector<std::pair<BodyCell*, DecodeContext>> activeSet_;
//	std::map<BodyPart*, std::pair<JointPivot*, CumulativeValue>> mapJointOffsets_;	// maps a body part pointer to its upstream joint
//																	// and relative genome offset of the joint (if joint exists)
	std::vector<Muscle*> muscles_;
	std::vector<VMSEntry<Neuron*>> vmsNeurons_;				// holds VMS locations for each neuron
	std::map<const Gene*, std::set<float>> neuralGenes_;	// first is neural gene, second is a set of VMS offsets from the decode context
															// the same neural gene is only interpreted multiple times if it appears in a different vms offset context
	std::set<const Gene*> bodyAttribGenes_;					// hold body attribute genes here and decode them when all genome is processed
#ifdef DEBUG
//	std::map<Neuron*, int> mapNeuronVirtIndex_;	// maps neurons to their virtual indices
//	std::map<InputSocket*, std::pair<std::string, int>> mapSockMotorInfo;	// first: motorName, second: inputID
#endif
//	std::map<uint64_t, SynapseInfo> mapSynapses_;
//	std::set<int> outputNeurons_;	// virtual indices of output neurons
//	std::set<int> inputNeurons_;	// virtual indices of input neurons
	std::vector<IMotor*> motors_;
	int nMotorLines_ = 0;
	std::vector<ISensor*> sensors_;
	std::map<InputSocket*, int> mapInputNerves_;	// maps inputSockets from motors to motor line indexes

	void decodeGene(Gene const& g, BodyCell &cell, DecodeContext &ctx, bool deferNeural);
	void decodeProtein(GeneProtein const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeOffset(GeneOffset const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeDivisionParam(GeneDivisionParam const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeJointAttrib(GeneJointAttribute const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeMuscleAttrib(GeneMuscleAttribute const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeVMSOffset(GeneVMSOffset const& g, BodyCell &cell, DecodeContext &ctx);
	void decodePartAttrib(GeneAttribute const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeSynapse(GeneSynapse const& g);
	void decodeTransferFn(GeneTransferFunction const& g);
	void decodeNeuralBias(GeneNeuralBias const& g);
	void decodeNeuralParam(GeneNeuralParam const& g);
//	bool partMustGenerateJoint(BodyPartType part_type);
//	void growBodyPart(BodyPart* parent, unsigned attachmentSegment, glm::vec4 hyperPosition, unsigned genomeOffset);
	void addMotor(IMotor* motor, BodyPart* part);
	void addSensor(ISensor* sensor);
	void resolveMuscleLinkage();
//	JointPivot* findNearestJoint(Muscle* m, int dir);
	void processLocalNeuralGenes(BodyCell& cell, DecodeContext &ctx);

	void decodeDeferredGenes();
	void specializeCells(bool &hasMouth, bool &hasEggLayer);
//	void checkAndAddNeuronMapping(int virtualIndex);
//	void updateNeuronConstant(int virtualIndex, float constant);
//	bool hasNeuron(int virtualIndex, bool physical); // checks whether a virtual neuron exists and, if requested, its physical equivalent too
	// Compute a synapse key (unique id for from-to pair:
//	inline uint64_t synKey(uint64_t from, uint64_t to) { return ((from << 32) & 0xFFFFFFFF00000000) | (to & 0xFFFFFFFF); }
//	void createSynapse(int from, int to, SynapseInfo const& info);
	void resolveNerveLinkage();
//	void commitNeurons();
	void linkMotorNerves(std::vector<VMSEntry<Neuron*>> const& orderedOutputNeurons_,
						 std::vector<VMSEntry<InputSocket*>> const& orderedMotorInputs_);
	void linkSensorNerves(std::vector<VMSEntry<Neuron*>> const& orderedInputNeurons_,
						  std::vector<VMSEntry<OutputSocket*>> orderedSensorOutputs_);

	// searches for the nerve nearest to the given matchCoordinate in the Virtual Matching Space; returns its index or -1 if none found
	template<typename T>
	int getVMSNearestObjectIndex(std::vector<VMSEntry<T>> const& entries, float matchCoord);	// returns -1 if none found

	template<typename T>
	void sortEntriesByVMSCoord(std::vector<VMSEntry<T>> &entries);

	// returns true if the gene applies to this cell (gene passes the branch restriction of the cell)
	bool geneQualifies(Gene& g, BodyCell& c);

	void updateCellDensity(BodyCell &cell);
	BodyPartType specializationType(BodyCell const& c) const;

	void cleanUp();
};

#endif //__ribosome_h__
