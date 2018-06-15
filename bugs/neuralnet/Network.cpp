#include "Network.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

NeuralNet::NeuralNet()
{
}

NeuralNet::~NeuralNet() {
	for (unsigned i=0,n=neurons.size(); i<n; ++i)
		delete neurons[i];
	neurons.clear();
}

void NeuralNet::iterate(float dt)
{
	// we split the update and push in two separate steps so that the order of the neurons will have no effect
	// on the speed at which data travels through synapses

	// step 1: update all neurons values before any data moves around
	for (Neuron* n : neurons)
		n->update_value(dt);
	// step 2: push all neurons output data simultaneously
	for (Neuron* n : neurons)
		n->push_output();
}
