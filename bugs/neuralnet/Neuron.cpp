#include "Neuron.h"
#include "InputSocket.h"
#include "../utils/rand.h"
#include "../utils/assert.h"
#include "../math/math2D.h"
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

void Neuron::addInput(std::unique_ptr<InputSocket> &&input, float priority) {
	assertDbg(pInputPriorities && "Attempting to add input after commit!!!");
	inputs.push_back(std::move(input));
	pInputPriorities->push_back(priority);
}
void Neuron::commitInputs() {
	assertDbg(pInputPriorities && "Only call this once at the end!!!");
	// sort inputs
	for (uint i=0; i+1<inputs.size(); i++) {
		for (uint j=i+1; j<inputs.size(); j++) {
			if ((*pInputPriorities)[i] < (*pInputPriorities)[j]) {
				xchg((*pInputPriorities)[i], (*pInputPriorities)[j]);
				xchg(std::move(inputs[i]), std::move(inputs[j]));
			}
		}
	}
	// delete priority data:
	delete pInputPriorities;
	pInputPriorities = nullptr;
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
	float cmdSignal = inputs.size() ? inputs[0]->value * inputs[0]->weight : 0;
	value += inputBias;
	value = transfFunc(value, neuralParam, cmdSignal, inputBias);
	if (std::isnan(value))
		value = 0;
}
