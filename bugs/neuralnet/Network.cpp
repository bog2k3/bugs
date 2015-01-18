#include "Network.h"

#include "../math/tools.h"
#include "Traverser.h"

#include <map>
using namespace std;

NeuralNet::NeuralNet()
{
}

NeuralNet::~NeuralNet() {
	for (unsigned i=0,n=neurons.size(); i<n; ++i)
		delete neurons[i];
	neurons.clear();
	inputs.clear();
	outputs.clear();
}

void NeuralNet::iterate()
{
	// step 1 and above: move from the first layer of neurons up the chain, until all neurons are visited
	Traverser trav(this);
	do {
		trav.getNextLayer(crtLayer);
		for (auto itN=crtLayer.begin(); itN != crtLayer.end(); ++itN) {
			Neuron* pNeuron = *itN;
			pNeuron->update_value();
			pNeuron->push_output();
		}
	} while (!crtLayer.empty());
}
