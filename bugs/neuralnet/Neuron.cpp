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
	inputs_.clear();
}

void Neuron::setTranferFunction(transferFuncNames fn) {
	transfFunc_ = mapTransferFunctions[fn];

	// check for special functions:
	if (   fn == transferFuncNames::FN_GATE
		|| fn == transferFuncNames::FN_MODULATE
		) {
		isZeroCmdSignal_ = true;
	}
}

void Neuron::addInput(std::unique_ptr<InputSocket> &&input, float priority) {
	assertDbg(pInputPriorities_ && "Attempting to add input after commit!!!");
	inputs_.push_back(std::move(input));
	pInputPriorities_->push_back(priority);
}
void Neuron::commitInputs() {
	assertDbg(pInputPriorities_ && "Only call this once at the end!!!");
	// sort inputs
	for (uint i=0; i+1<inputs_.size(); i++) {
		for (uint j=i+1; j<inputs_.size(); j++) {
			if ((*pInputPriorities_)[i] < (*pInputPriorities_)[j]) {
				xchg((*pInputPriorities_)[i], (*pInputPriorities_)[j]);
				xchg(std::move(inputs_[i]), std::move(inputs_[j]));
			}
		}
	}
	// delete priority data:
	delete pInputPriorities_;
	pInputPriorities_ = nullptr;
}

void Neuron::update_value()
{
	value_ = 0;
	for (unsigned i=0, n=inputs_.size(); i<n; ++i) {
		if (isZeroCmdSignal_ && !i) {	// don't use the special command input to compute the output value
			continue;
		}
		value_ += inputs_[i]->value * inputs_[i]->weight;
	}
	float cmdSignal = inputs_.size() ? inputs_[0]->value * inputs_[0]->weight : 0;
	value_ += inputBias;
	value_ = transfFunc_(value_, neuralParam, cmdSignal, inputBias);
	if (std::isnan(value_))
		value_ = 0;
}
