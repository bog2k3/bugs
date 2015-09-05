/*
 *	a ribosome decodes a sequence of chromosomes and builds a fully-functional neural network
 */

#ifndef __ribosome_h__
#define __ribosome_h__

#include "Genome.h"
#include "Gene.h"
#include "CummulativeValue.h"
#include "../body-parts/BodyPart.h"
#include <glm/vec4.hpp>
#include <vector>
#include <map>

class Bug;
class BodyPart;
class Gene;
class Neuron;

struct NeuronInfo {
	int index;
	CummulativeValue transfer;
	CummulativeValue constant;
	CummulativeValue inputVMSCoord = 0;
	CummulativeValue outputVMSCoord = 0;
	NeuronInfo(int index, float transfer, float constant)
		: index(index), transfer(transfer), constant(constant) {
	}
	explicit NeuronInfo(int index)
		: index(index) {
	}
	NeuronInfo(NeuronInfo const& other)
		: index(other.index), transfer(other.transfer), constant(other.constant) {
	}
	NeuronInfo() : index(-1) {
	}
};

struct GrowthData {
	int startGenomePos; // initial genome offset for this part (children are relative to this one)
	int crtGenomePos; // current READ position in genome for this part
	glm::vec4 hyperPositions[MAX_CHILDREN] {glm::vec4(0)};	// holds hyper-space positions for each segment in a body part
	CummulativeValue offsets[MAX_CHILDREN]; // holds relative genome offsets for each segment in a body part

	GrowthData(int initialOffs)
		: startGenomePos(initialOffs), crtGenomePos(initialOffs) {
	}
};

template<typename T>
using inputOutputNerve = std::pair<T, float>;	// first (T) is the nerve pointer, second is the VMS coordinate

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

private:
	Bug* bug_;
//	int nDefaultSensors = 0;
	std::vector<Gene*> neuralGenes;
	std::vector<std::pair<BodyPart*, GrowthData>> activeSet_;
	// std::map<GeneCommand*, int> mapGeneToIterations_; // maps growth genes to number of iterations (how many times they've been read so far)
	std::map<int, NeuronInfo> mapNeurons_;	// maps virtual neuron indices (as encoded in the genes)
											// to actual indices in the neural network plus cummulative properties
	std::map<int64_t, CummulativeValue> mapSynapses;
	std::vector<inputOutputNerve<Neuron*>> orderedOutputNeurons_;	// output neurons ordered by their VMS coordinate (smallest to greatest)
	std::vector<inputOutputNerve<Neuron*>> orderedInputNeurons_;	// input neurons ordered by their VMS coordinate (smallest to greatest)
	std::vector<inputOutputNerve<OutputSocket*>> orderedSensorOutputs_;	// sensors ordered by their VMS coordinate (smallest to greatest)

	void decodeGene(Gene &g, BodyPart* part, GrowthData *growthData, bool deferNeural);
	void decodeProtein(GeneProtein &g, BodyPart* part, GrowthData *growthData);
	void decodeOffset(GeneOffset &g, BodyPart* part, GrowthData *growthData);
	void decodePartAttrib(GeneAttribute const& g, BodyPart* part);
	void decodeSynapse(GeneSynapse const& g);
	void decodeTransferFn(GeneTransferFunction const& g);
	void decodeNeuralConst(GeneNeuralConstant const& g);
	void decodeNeuronOutputCoord(GeneNeuronOutputCoord const& g);
	void decodeNeuronInputCoord(GeneNeuronInputCoord const& g);
	bool partMustGenerateJoint(int part_type);
	void growBodyPart(BodyPart* parent, int attachmentSegment, glm::vec4 hyperPosition, int genomeOffset);

	void initializeNeuralNetwork();
	void decodeDeferredGenes();
	void checkAndAddNeuronMapping(int virtualIndex);
	void updateNeuronTransfer(int virtualIndex, float transfer);
	void updateNeuronConstant(int virtualIndex, float constant);
	void updateNeuronOutputCoord(int virtualIndex, float VMScoord);
	void updateNeuronInputCoord(int virtualIndex, float VMScoord);
	inline bool hasNeuron(int virtualIndex) { return mapNeurons_.find(virtualIndex) != mapNeurons_.end(); }
	// Compute a synapse key (unique id for from-to pair:
	inline int64_t synKey(int64_t from, int64_t to) { return ((from << 32) & 0xFFFFFFFF00000000) | (to & 0xFFFFFFFF); }
	void createSynapse(int from, int to, int commandNeuronsOfs, float weight);
	void linkMotorNerves();
	void linkSensorNerves();

	// searches for the nerve nearest to the given matchCoordinate in the Virtual Matching Space; returns its index or -1 if none found
	template<typename T>
	int getVMSNearestNerveIndex(std::vector<inputOutputNerve<T>> nerves, float matchCoord);	// returns -1 if none found

	template<typename T>
	void sortNervesByVMSCoord(std::vector<inputOutputNerve<T>> nerves);

	void cleanUp();
};

#endif //__ribosome_h__
