#include "Neuron.h"
#include "../utils/rand.h"
#include <cassert>
#include "InputSocket.h"

using namespace std;

Neuron::Neuron()
	: transfFunc(transfer_fn_one)
	, value(0)
	, neuralConstant(0)
	, RID(0)
{
}

Neuron::~Neuron() {
	inputs.clear();
}

void Neuron::update_value()
{
	value = 0;
	for (unsigned i=0, n=inputs.size(); i<n; ++i) {
		value += inputs[i]->value * inputs[i]->weight;
	}
}

void Neuron::push_output()
{
	output.push_value(transfFunc(value, neuralConstant));
}

void Neuron::retrieve_targets(unsigned long opRID, std::vector<Neuron*> &out_targets)
{
	vector<InputSocket*>& list_targets = output.getTargets();
	for (auto it=list_targets.begin(); it != list_targets.end(); ++it) {
		InputSocket* pOtherInput = *it;
		if (pOtherInput->pParentNeuron != NULL && pOtherInput->pParentNeuron->RID != opRID) {
			pOtherInput->pParentNeuron->RID = opRID;
			out_targets.push_back(pOtherInput->pParentNeuron);
		}
	}
}
