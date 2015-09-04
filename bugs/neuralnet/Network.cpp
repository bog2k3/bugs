#include "Network.h"
#include "Traverser.h"

NeuralNet::NeuralNet()
//	: pTraverser(new Traverser(this))
{
}

NeuralNet::~NeuralNet() {
	for (unsigned i=0,n=neurons.size(); i<n; ++i)
		delete neurons[i];
	neurons.clear();
//	inputs.clear();
//	outputs.clear();
//	delete pTraverser;
}

void NeuralNet::iterate()
{
//	pTraverser->reset();
	// step 1 and above: move from the first layer of neurons up the chain, until all neurons are visited
	for (Neuron* n : neurons)
	{
//		pTraverser->getNextLayer(crtLayer);
//		for (auto itN=crtLayer.begin(); itN != crtLayer.end(); ++itN) {
//			Neuron* pNeuron = *itN;
			n->update_value();
			n->push_output();
//		}
	}
//	} while (!crtLayer.empty());
}
