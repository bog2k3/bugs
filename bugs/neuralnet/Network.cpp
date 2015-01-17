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

void NeuralNet::iterate(std::vector<double> input_values, std::vector<double> &output_values, int requiredOutputs)
{
	// step 1: push values into inputs
	auto it = inputs.begin(), itE = inputs.end();
	for (unsigned int i=0; it != itE; ++it, ++i) {
		double val_i = i < input_values.size() ? input_values[i] : 0;
		(*it)->push_value(val_i);
	}

	// step 2 and above: move from the first layer of neurons up the chain, until all neurons are visited
	Traverser trav(this);
	vector<Neuron*> crtLayer;
	do {
		crtLayer = trav.getNextLayer();
		vector<Neuron*>::iterator itN = crtLayer.begin(), itNE = crtLayer.end();
		for (; itN != itNE; ++itN) {
			Neuron* pNeuron = *itN;
			pNeuron->update_value();
			pNeuron->push_output();
		}
	} while (!crtLayer.empty());

	// step 3 : all neurons visited, copy the output values:
	for (int i=0; i<requiredOutputs; ++i) {
		output_values.push_back(outputs[i]->value);
	}
}
