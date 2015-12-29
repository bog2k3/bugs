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

class NeuralNet {
public:
	NeuralNet();
	~NeuralNet();

	// performs one data iteration - new values are computed then output values are pushed through synapses.
	void iterate();

	std::vector<Neuron*> neurons;
};

#endif //__network_h__
