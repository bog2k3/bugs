#include "Network.h"

#include "../math/tools.h"
#include "Traverser.h"

#include <map>
using namespace std;

NeuralNet::NeuralNet()
{
}

// clone the entire network with all neurons and synapses and stuff, updating all pointers
NeuralNet::NeuralNet(const NeuralNet& original)
{
	map<Input*, Input*> mapInputs; // map for remapping inputs for the new network
	
	for (unsigned i=0, n=original.outputs.size(); i<n; ++i) {
		Input* pNewInput = new Input(NULL);
		outputs.push_back(pNewInput);

		mapInputs[original.outputs[i]] = pNewInput;
	}

	for (unsigned i=0, n=original.neurons.size(); i<n; ++i) {
		Neuron* pNewNeuron = new Neuron(*original.neurons[i]);
		neurons.push_back(pNewNeuron);

		for (unsigned j=0, nj=original.neurons[i]->inputs.size(); j<nj; ++j)
			mapInputs[original.neurons[i]->inputs[j]] = pNewNeuron->inputs[j];
	}

	for (unsigned i=0, n=original.inputs.size(); i<n; ++i) {
		OutputSocket* pNewOutputSock = new OutputSocket(*original.inputs[i]);
		inputs.push_back(pNewOutputSock);
		vector<Input*> &outputTargets = pNewOutputSock->getTargets();
		for (unsigned j=0, nj=outputTargets.size(); j<nj; ++j) {
			outputTargets[j] = mapInputs[outputTargets[j]];	// remap to new neurons' inputs
		}
	}

	for (unsigned i=0, n=neurons.size(); i<n; ++i) {
		vector<Input*> &outputTargets = neurons[i]->output.getTargets();
		for (unsigned j=0, nj=outputTargets.size(); j<nj; ++j)
			outputTargets[j] = mapInputs[outputTargets[j]]; // remap Neuron's targets to new neurons
	}
}

NeuralNet::~NeuralNet() {
	for (unsigned i=0,n=neurons.size(); i<n; ++i)
		delete neurons[i];
	neurons.clear();
	for (unsigned i=0,n=inputs.size(); i<n; ++i)
		delete inputs[i];
	inputs.clear();
	for (unsigned i=0,n=outputs.size(); i<n; ++i)
		delete outputs[i];
	outputs.clear();
}

void NeuralNet::iterate(std::vector<double> input_values, std::vector<double> &output_values, int requiredOutputs)
{
	// step 1: push values into inputs
	vector<OutputSocket*>::iterator it = inputs.begin(),
		itE = inputs.end();
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
