#include "Traverser.h"
#include "Network.h"
#include "Neuron.h"
#include "../math/tools.h"

#include <vector>
#include <assert.h>

#include "InputSocket.h"
using namespace std;

Traverser::Traverser(NeuralNet* pNet)
	: RID(new_RID())
	, pNetwork(pNet)
	, finished(false)
{
#pragma message("WARNING : traversals must be exclusive due to Neuron's RID being shared by all contexts. Must use common mutex on constructor")
	assert(pNetwork != NULL);
}

Traverser::~Traverser() {
}

void Traverser::getNextLayer(vector<Neuron*> &out)
{
	out.clear();

	if (finished) {
		return;
	}

	if (crt_neurons.empty()) {
		// THIS IS THE FIRST STEP, we take the NeuralNet inputs and move to their targets
		auto it = pNetwork->inputs.begin(), itE = pNetwork->inputs.end();
		// step 1: get first round of neurons
		for (unsigned int i=0; it != itE; ++it, ++i) {
			OutputSocket* input_i = it->get();
			vector<InputSocket*> &list_attached_ins = input_i->getTargets();
			vector<InputSocket*>::iterator itA = list_attached_ins.begin(),
				itAE = list_attached_ins.end();
			for (; itA != itAE; ++itA) {
				InputSocket* att_in = *itA;
				if (att_in->pParentNeuron->RID != RID) {
					att_in->pParentNeuron->RID = RID;
					crt_neurons.push_back(att_in->pParentNeuron);
				}
			}
		}
		// also add all neurons that don't have inputs
		for (int i=0; i<pNetwork->neurons.size(); i++)
			if (pNetwork->neurons[i]->inputs.size() == 0)
				crt_neurons.push_back(pNetwork->neurons[i]);
	} else {
		// step 2 and above: move from the current list of neurons up the chain, until all neurons are visited
		vector<Neuron*> next_neurons;
		vector<Neuron*> temp_list;
		for (auto itN=crt_neurons.begin(); itN != crt_neurons.end(); ++itN) {
			Neuron* pNeuron = *itN;
			temp_list.clear();
			pNeuron->retrieve_targets(RID, temp_list);
			next_neurons.insert(next_neurons.end(), temp_list.begin(), temp_list.end());
		}
		crt_neurons.swap(next_neurons);
		if (crt_neurons.empty()) {
			finished = true;
		}
	}
	out.insert(out.end(), crt_neurons.begin(), crt_neurons.end());
}

vector<Neuron*> Traverser::getIsolatedNeurons()
{
	if (!finished)
		return vector<Neuron*>();

	vector<Neuron*> isolated;

	for (unsigned i=0, n=pNetwork->neurons.size(); i<n; i++) {
		if (pNetwork->neurons[i]->RID != RID) {
			isolated.push_back(pNetwork->neurons[i]);
		}
	}
	return isolated;
}
