#include "Neuron.h"
#include "../utils/rand.h"
#include "InputSocket.h"
#include <cassert>
#include <cmath>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

using namespace std;

Neuron::Neuron() {
}

Neuron::~Neuron() {
	inputs.clear();
}

void Neuron::update_value()
{
	value = 0;
	for (unsigned i=0, n=inputs.size(); i<n; ++i) {
		if (isZeroCmdSignal && !i) {	// don't use the special command input to compute the output value
			continue;
		}
		value += inputs[i]->value * inputs[i]->weight;
	}
	float cmdSignal = inputs[0]->value * inputs[0]->weight;
	if (!isZeroCmdSignal)	// for special neurons, such as gate or modulator we don't add the bias to the sum of inputs
		value += inputBias;
	value = transfFunc(cmdSignal, value, param);
	if (std::isnan(outVal))
		outVal = 0;
}

//void Neuron::retrieve_targets(unsigned long opRID, std::vector<Neuron*> &out_targets)
//{
//	vector<InputSocket*>& list_targets = output.getTargets();
//	for (auto it=list_targets.begin(); it != list_targets.end(); ++it) {
//		InputSocket* pOtherInput = *it;
//		if (pOtherInput->pParentNeuron != NULL && pOtherInput->pParentNeuron->RID != opRID) {
//			pOtherInput->pParentNeuron->RID = opRID;
//			out_targets.push_back(pOtherInput->pParentNeuron);
//		}
//	}
//}
