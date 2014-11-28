/*
 *	a neural network represents a number of interconnected neurons,
 *	with precisely defined inputs and outputs
 */

#ifndef __network_h__
#define __network_h__

#include <vector>
#include "../genetics/Chromosome.h"
#include "Neuron.h"
#include "OutputSocket.h"

class NeuralNet {
public:

#define MAX_NETWORK_DEFAULT_INPUTS 16

	// create and develop a new network from the given genome
	NeuralNet(Genome theGenome);

	// clone the entire network with all neurons and synapses and stuff, updating all pointers
	NeuralNet(const NeuralNet& original);

	~NeuralNet();

	// feeds the inputs into the network and performs one calculus iteration over all neurons, generating
	// a new set of outputs; the order in which neurons are given action, is determined from the normal
	// flow of data through the network, from the inputs toward the outputs; this makes feedback loops
	// and any kind of delayed synapse to work correctly.
	void iterate(std::vector<double> inputs_values, std::vector<double> &outputs_values, int requiredOutputs);

	std::vector<Neuron*> neurons;
	std::vector<OutputSocket*> inputs; // list of input sockets that feed data to neurons
	std::vector<Input*> outputs; // list of output sockets that output data from the network

	const Genome& getGenome() { return genome; }
protected:
	Genome genome;

	friend class Ribosome;
};

#endif //__network_h__
