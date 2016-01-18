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

void Neuron::setTranferFunction(transferFuncNames fn) {
	transfFunc = mapTransferFunctions[fn];

	// check for special functions:
	if (   fn == transferFuncNames::FN_GATE
		|| fn == transferFuncNames::FN_MODULATE
		) {
		isZeroCmdSignal = true;
	}
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
	value += inputBias;
	value = transfFunc(value, neuralParam, cmdSignal, inputBias);
	if (std::isnan(value))
		value = 0;
}
