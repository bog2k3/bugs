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

struct NeuronInfo {
	int index;
	CummulativeValue transfer;
	CummulativeValue constant;
	CummulativeValue inputVMSCoord;
	CummulativeValue outputVMSCoord;
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
	unsigned startGenomePos; // initial genome offset for this part (children are relative to this one)
	unsigned crtGenomePos; // current READ position in genome for this part
	glm::vec4 hyperPositions[BodyPart::MAX_CHILDREN] { glm::vec4() };	// holds hyper-space positions for each segment in a body part
	CummulativeValue offsets[BodyPart::MAX_CHILDREN]; // holds relative genome offsets for each segment in a body part

	GrowthData(int initialOffs)
		: startGenomePos(initialOffs), crtGenomePos(initialOffs) {
	}
};

template<typename T>
using InputOutputNerve = std::pair<T, float>;	// first (T) is the nerve pointer, second is the VMS coordinate

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

private:
	Bug* bug_;
	std::vector<std::pair<BodyPart*, GrowthData>> activeSet_;
	std::map<BodyPart*, std::pair<Joint*, CummulativeValue>> mapJointOffsets_;	// maps a body part pointer to its upstream joint
																	// and relative genome offset of the joint (if joint exists)
	std::vector<Muscle*> muscles_;
	std::vector<const Gene*> neuralGenes_;
	std::map<int, NeuronInfo> mapNeurons_;	// maps virtual neuron indices (as encoded in the genes)
											// to actual indices in the neural network plus cummulative properties
#ifdef DEBUG
	std::map<Neuron*, int> mapNeuronVirtIndex_;	// maps neurons to their virtual indices
	std::map<InputSocket*, std::pair<std::string, int>> mapSockMotorInfo;	// first: motorName, second: inputID
#endif
	std::map<int64_t, CummulativeValue> mapSynapses_;
	std::set<int> outputNeurons_;	// virtual indices of output neurons
	std::set<int> inputNeurons_;	// virtual indices of input neurons
	std::vector<IMotor*> motors_;
	int nMotorLines_ = 0;
	std::vector<ISensor*> sensors_;
	std::map<InputSocket*, int> mapInputNerves_;	// maps inputSockets from motors to motor line indexes

	void decodeGene(Gene const& g, BodyPart* part, GrowthData *growthData, bool deferNeural);
	void decodeProtein(GeneProtein const& g, BodyPart* part, GrowthData *growthData);
	void decodeOffset(GeneOffset const& g, BodyPart* part, GrowthData *growthData);
	void decodeJointOffset(GeneJointOffset const& g, BodyPart* part);
	void decodePartAttrib(GeneAttribute const& g, BodyPart* part);
	void decodeSynapse(GeneSynapse const& g);
	void decodeTransferFn(GeneTransferFunction const& g);
	void decodeNeuralConst(GeneNeuralConstant const& g);
	void decodeNeuronOutputCoord(GeneNeuronOutputCoord const& g);
	void decodeNeuronInputCoord(GeneNeuronInputCoord const& g);
	bool partMustGenerateJoint(BodyPartType part_type);
	void growBodyPart(BodyPart* parent, unsigned attachmentSegment, glm::vec4 hyperPosition, unsigned genomeOffset);
	void addMotor(IMotor* motor, BodyPart* part);
	void addSensor(ISensor* sensor);
	void resolveMuscleLinkage();
	Joint* findNearestJoint(Muscle* m, int dir);

	void initializeNeuralNetwork();
	void decodeDeferredGenes();
	void checkAndAddNeuronMapping(int virtualIndex);
	void updateNeuronTransfer(int virtualIndex, float transfer);
	void updateNeuronConstant(int virtualIndex, float constant);
	inline bool hasNeuron(int virtualIndex) { return mapNeurons_.find(virtualIndex) != mapNeurons_.end(); }
	// Compute a synapse key (unique id for from-to pair:
	inline int64_t synKey(int64_t from, int64_t to) { return ((from << 32) & 0xFFFFFFFF00000000) | (to & 0xFFFFFFFF); }
	void createSynapse(int from, int to, float weight);
	void resolveNerveLinkage();
	void linkMotorNerves(std::vector<InputOutputNerve<Neuron*>> const& orderedOutputNeurons_,
						 std::vector<InputOutputNerve<InputSocket*>> const& orderedMotorInputs_);
	void linkSensorNerves(std::vector<InputOutputNerve<Neuron*>> const& orderedInputNeurons_,
						  std::vector<InputOutputNerve<OutputSocket*>> orderedSensorOutputs_);

	// searches for the nerve nearest to the given matchCoordinate in the Virtual Matching Space; returns its index or -1 if none found
	template<typename T>
	int getVMSNearestNerveIndex(std::vector<InputOutputNerve<T>> const& nerves, float matchCoord);	// returns -1 if none found

	template<typename T>
	void sortNervesByVMSCoord(std::vector<InputOutputNerve<T>> &nerves);

	void cleanUp();
};

#endif //__ribosome_h__
