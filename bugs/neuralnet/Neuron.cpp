#include "Neuron.h"

#include "../math/tools.h"
#include <assert.h>

using namespace std;

Neuron::Neuron()
	: transfFunc(NULL)
	, value(0)
	, neuralConstant(0)
	, RID(0)
{
}

Neuron::~Neuron() {
	for (unsigned i=0, n=inputs.size(); i<n; ++i)
		delete inputs[i];
	inputs.clear();
}

void Input::push(float value)
{
	this->value = value;
}

void Neuron::update_value()
{
	float* input_array = new float[inputs.size()];
	float* weight_array = new float[inputs.size()];
	for (unsigned i=0, n=inputs.size(); i<n; ++i) {
		input_array[i] = inputs[i]->value;
		weight_array[i] = inputs[i]->weight;
	}
	// compute value:
	value = compute_sum(inputs.size(), input_array, weight_array);
	delete [] input_array;
	delete [] weight_array;
}

float Neuron::compute_sum(int count, float input_array[], float weight_array[])
{
	float s = 0;
	for (int i=0; i<count; i++)
		s += input_array[i] * weight_array[i];
	return s;
}

void Neuron::push_output()
{
	output.push_value(transfFunc(value, neuralConstant));
}

void Neuron::retrieve_targets(unsigned long opRID, std::vector<Neuron*> &out_targets)
{
	vector<Input*>& list_targets = output.getTargets();
	vector<Input*>::iterator it = list_targets.begin(),
		it_e = list_targets.end();
	for (; it != it_e; ++it) {
		Input* pOtherInput = *it;
		if (pOtherInput->pParentNeuron != NULL && pOtherInput->pParentNeuron->RID != opRID) {
			pOtherInput->pParentNeuron->RID = opRID;
			out_targets.push_back(pOtherInput->pParentNeuron);
		}
	}
}
