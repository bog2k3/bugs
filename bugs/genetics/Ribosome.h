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
class Joint;
class JointPivot;
class Cell;

struct NeuronInfo {
	Neuron* neuron;
	CumulativeValue transfer;
	CumulativeValue bias;
	CumulativeValue param;

	explicit NeuronInfo(Neuron* n)
		: neuron(n) {
	}
	NeuronInfo(NeuronInfo const& other) = default;
	NeuronInfo() : neuron(nullptr) {
	}
};

struct SynapseInfo {
	CumulativeValue weight;
	CumulativeValue priority;
	Joint* ownerJoint;	// this may be null if the synapse is between neurons/sensors within the same body part
};

template<typename T>
using VMSEntry = std::pair<T, float>;	// first (T) is the object (neuron, input/output socket), second is the VMS coordinate

struct DecodeContext {
	unsigned startGenomePos = 0; // initial genome offset for this cell (children are relative to this one)
	unsigned crtGenomePos = 0; // current READ position in genome for this cell
	CumulativeValue childOffsets[2]; // holds relative genome offsets for each future child cell (0 is left, 1 right)
	CumulativeValue vmsOffset;
	float parentVmsOffset = 0;
	std::set<const GeneVMSOffset*> vmsOffsetGenes;	// all vms offset genes are added to this set and passed down to the cell's children
											// if the same vms offset gene is encountered by the child cell it will be ignored
											// thus the vms offset gene can affect a cell only once (it is inherited by the children)
	std::vector<const Gene*> neuralGenes;
	std::vector<VMSEntry<NeuronInfo>> vmsNeurons_;			// holds VMS locations and cumulative attriutes for each neuron
	std::vector<ISensor*> sensors_;			// sensors within the cell

	DecodeContext() = default;

	DecodeContext(int initialOffs)
		: startGenomePos(initialOffs), crtGenomePos(initialOffs) {
	}

	DecodeContext(const DecodeContext& c) = default;

	DecodeContext(DecodeContext &&c)
	{
		operator=(std::move(c));
	}

	DecodeContext& operator = (DecodeContext &&c) {
		startGenomePos = c.startGenomePos;
		crtGenomePos = c.crtGenomePos;
		vmsOffset = c.vmsOffset;
		parentVmsOffset = c.parentVmsOffset;
		childOffsets[0] = c.childOffsets[0];
		childOffsets[1] = c.childOffsets[1];
		vmsOffsetGenes.swap(c.vmsOffsetGenes);
		neuralGenes.swap(c.neuralGenes);
		vmsNeurons_.swap(c.vmsNeurons_);
		return *this;
	}
};

/**
 * decodes the entity's genome and builds it step by step. When finished the entity will have its final
 * shape and preprogrammed functionality, but will be very small in size.
 */
class Ribosome {
public:
	Ribosome(Bug* the_bug);
	~Ribosome();

	/**
	 * develops the entity one more step. Returns true as long as the process is not finished.
	 */
	bool step();

	void drawCells(Viewport* vp);

	bool isPreFinalStep() const { return activeSet_.empty(); }

private:
	Bug* bug_;
	std::vector<BodyCell*> cells_;
	std::vector<std::pair<BodyCell*, DecodeContext>> activeSet_;
	std::map<BodyCell*, DecodeContext> cellContext_;		// hold decode data for each specialized cell
	std::map<Cell*, BodyPart*> cellToPart_;				// associate each specialized or joint cell with the corresponding body part
	std::set<const Gene*> bodyAttribGenes_;					// hold body attribute genes here and decode them when all genome is processed
	std::map<IMotor*, DecodeContext*> motors_;

	void postDecodeAndFinalization();		// does post-decode operations (deferred genes) and cell specialization

	void decodeGene(Gene const& g, BodyCell &cell, DecodeContext &ctx, bool deferNeural);
	void decodeProtein(GeneProtein const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeOffset(GeneOffset const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeDivisionParam(GeneDivisionParam const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeJointAttrib(GeneJointAttribute const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeMuscleAttrib(GeneMuscleAttribute const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeVMSOffset(GeneVMSOffset const& g, BodyCell &cell, DecodeContext &ctx);
	void decodePartAttrib(GeneAttribute const& g, BodyCell &cell, DecodeContext &ctx);
	void decodeNeuralGene(Gene const& g, float vmsOffset, float vmsDirection,
			std::vector<VMSEntry<std::pair<OutputSocket*, BodyCell*>>> &outSockets,
			std::vector<VMSEntry<NeuronInfo>> &vmsNeurons,
			std::map<std::pair<OutputSocket*, Neuron*>, SynapseInfo> &mapSynapses);
	void decodeSynapse(GeneSynapse const& g, float vmsOffset, float vmsDirection,
			std::vector<VMSEntry<std::pair<OutputSocket*, BodyCell*>>> &outSockets,
			std::vector<VMSEntry<NeuronInfo>> &vmsNeurons,
			std::map<std::pair<OutputSocket*, Neuron*>, SynapseInfo> &mapSynapses);
	void decodeTimeSynapse(GeneTimeSynapse const& g, float vmsOffset, float vmsDirection,
			std::vector<VMSEntry<NeuronInfo>> &vmsNeurons,
			std::map<std::pair<OutputSocket*, Neuron*>, SynapseInfo> &mapSynapses);
	void decodeTransferFn(GeneTransferFunction const& g, float vmsOffset, float vmsDirection,
			std::vector<VMSEntry<NeuronInfo>> &vmsNeurons);
	void decodeNeuralBias(GeneNeuralBias const& g, float vmsOffset, float vmsDirection,
			std::vector<VMSEntry<NeuronInfo>> &vmsNeurons);
	void decodeNeuralParam(GeneNeuralParam const& g, float vmsOffset, float vmsDirection,
			std::vector<VMSEntry<NeuronInfo>> &vmsNeurons);
//	void addMotor(IMotor* motor, BodyPart* part);
	void createNeurons(BodyCell& cell, DecodeContext &ctx);
	void updateSynapseInfo(OutputSocket *from, NeuronInfo &to, Joint* synJoint,
			std::map<std::pair<OutputSocket*, Neuron*>, SynapseInfo> &mapSynapses,
			float priority, float weight);

	void decodeDeferredGenes();
	// builds and sorts by vms coord a vector of all the outputSockets from neurons and sensors:
	void buildOutputSocketsList(BodyCell* cell, std::vector<VMSEntry<std::pair<OutputSocket*, BodyCell*>>> &v);
	void specializeCells(bool &hasMouth, bool &hasEggLayer);
	void resolveMotorLinkage();
	void commitNeurons();
	void linkMotorNerves(std::vector<VMSEntry<NeuronInfo>> const& neurons, std::vector<VMSEntry<InputSocket*>> const& orderedMotorInputs_);

	// searches for the nerve nearest to the given matchCoordinate in the Virtual Matching Space; returns its index or -1 if none found
	template<typename T, class PredT>
	int getVMSNearestObjectIndex(std::vector<VMSEntry<T>> const& entries, float matchCoord,
			PredT validatePred, int start=0, int end=-1);	// returns -1 if none found

	template<typename T>
	void sortEntriesByVMSCoord(std::vector<VMSEntry<T>> &entries);

	// returns true if the gene applies to this cell (gene passes the branch restriction of the cell)
	bool geneQualifies(Gene& g, BodyCell& c);

	void updateCellDensity(BodyCell &cell);
	BodyPartType specializationType(BodyCell const& c) const;

	void cleanUp();
};

#endif //__ribosome_h__
