#include "Neuron.h"

#include "../math/tools.h"
#include <assert.h>

using namespace std;

Neuron::Neuron(int ID)
	: ID(ID)
	, transfFunc(NULL)
	, value(0)
	, bias(0)
	, transferConstant(0)
	, isInput(false)
	, isOutput(false)
	, RID(0)
{
}

Neuron::Neuron(const Neuron& original)
	: ID(original.ID)
	, transfFunc(original.transfFunc)
	, value(original.value)
	, bias(original.bias)
	, transferConstant(original.transferConstant)
	, isInput(original.isInput)
	, isOutput(original.isOutput)
	, RID(0)
	, output(original.output)
{
	// create inputs:
	for(unsigned i=0, n=original.inputs.size(); i<n; ++i) {
		Input* pIn = new Input(this);
		pIn->value = original.inputs[i]->value;
		pIn->weight = original.inputs[i]->weight;
		inputs.push_back(pIn);
	}
}

Neuron::~Neuron() {
	for (unsigned i=0, n=inputs.size(); i<n; ++i)
		delete inputs[i];
	inputs.clear();
}

void Input::push(double value)
{
	this->value = value;
}

void Neuron::update_value()
{
	double* input_array = new double[inputs.size()];
	double* weight_array = new double[inputs.size()];
	for (unsigned i=0, n=inputs.size(); i<n; ++i) {
		input_array[i] = inputs[i]->value;
		weight_array[i] = inputs[i]->weight;
	}
	// compute value:
	value = bias + compute_sum(inputs.size(), input_array, weight_array);
	delete [] input_array;
	delete [] weight_array;
}

double Neuron::compute_sum(int count, double input_array[], double weight_array[])
{
	double s = 0;
	for (int i=0; i<count; i++)
		s += input_array[i] * weight_array[i];
	return s;
}

void Neuron::push_output()
{
	output.push_value(transfFunc(value, transferConstant));
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
