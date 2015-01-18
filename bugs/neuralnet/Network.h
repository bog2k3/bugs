/*
 *	a neural network represents a number of interconnected neurons,
 *	with precisely defined inputs and outputs
 */

#ifndef __network_h__
#define __network_h__

#include "Neuron.h"
#include "OutputSocket.h"

#include <vector>
#include <memory>

class Traverser;

class NeuralNet {
public:
	NeuralNet();
	~NeuralNet();

	// feeds the inputs into the network and performs one calculus iteration over all neurons, generating
	// a new set of outputs; the order in which neurons are given action, is determined from the normal
	// flow of data through the network, from the inputs toward the outputs; this makes feedback loops
	// and any kind of delayed synapse to work correctly.
	void iterate();

	std::vector<Neuron*> neurons;
	std::vector<std::shared_ptr<OutputSocket>> inputs; // list of input sockets that feed data to neurons
	std::vector<std::shared_ptr<InputSocket>> outputs; // list of output sockets that output data from the network

private:
	std::vector<Neuron*> crtLayer;
	Traverser* pTraverser;
};

#endif //__network_h__
