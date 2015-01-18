/*
 *	a ribosome decodes a sequence of chromosomes and builds a fully-functional neural network
 */

#ifndef __ribosome_h__
#define __ribosome_h__

#include "Genome.h"
#include "Gene.h"
#include "CummulativeValue.h"
#include <vector>
#include <map>

class Bug;
class BodyPart;

struct NeuronInfo {
	int index;
	CummulativeValue transfer;
	CummulativeValue constant;
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
	unsigned crtPosition_;
	std::vector<const GeneGeneralAttribute*> generalAttribGenes;
	std::vector<const Gene*> neuralGenes;
	std::vector<BodyPart*> activeSet_;
	std::map<int, NeuronInfo> mapNeurons_;	// maps virtual neuron indices (as encoded in the genes)
											// to actual indices in the neural network plus cummulative properties
	std::map<int64_t, CummulativeValue> mapSynapses;
	std::map<int64_t, CummulativeValue> mapFeedbackSynapses;

	void decodeGene(Gene const& g, bool deferNeural);
	void decodeDevelopCommand(GeneCommand const& g);
	void decodeDevelopGrowth(GeneCommand const& g);
	void decodeDevelopSplit(GeneCommand const& g);
	void decodePartAttrib(GeneLocalAttribute const& g);
	void decodeGeneralAttrib(GeneGeneralAttribute const& g);
	void decodeSynapse(GeneSynapse const& g);
	void decodeFeedbackSynapse(GeneFeedbackSynapse const& g);
	void decodeTransferFn(GeneTransferFunction const& g);
	void decodeNeuralConst(GeneNeuralConstant const& g);
	bool partMustGenerateJoint(int part_type);

	void initializeNeuralNetwork();
	void decodeDeferredGenes();
	void checkAndAddNeuronMapping(int virtualIndex);
	void updateNeuronTransfer(int virtualIndex, float transfer);
	void updateNeuronConstant(int virtualIndex, float constant);
	inline bool hasNeuron(int virtualIndex) { return mapNeurons_.find(virtualIndex) != mapNeurons_.end(); }
	// Compute a synapse key (unique id for from-to pair:
	inline int64_t synKey(int64_t from, int64_t to) { return ((from << 32) & 0xFFFFFFFF00000000) | (to & 0xFFFFFFFF); }
	void createSynapse(int from, int to, int commandNeuronsOfs, float weight);
	void createFeedbackSynapse(int from, int to, int commandNeuronsOfs, float weight);

	void cleanUp();
};

#endif //__ribosome_h__
