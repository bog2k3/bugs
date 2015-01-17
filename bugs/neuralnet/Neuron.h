/*
 *	neuron
 */

#ifndef __neuron_h__
#define __neuron_h__

#include "functions.h"
#include "OutputSocket.h"
#include <vector>

class Neuron;

class Input {
public:
	float value;
	float weight;
	Neuron* pParentNeuron;

	Input(Neuron* pParentNeuron, float weight) :
		value(0), weight(weight), pParentNeuron(pParentNeuron) {
	}

	void push(float value); // pushes a new value into the input
};

class Neuron {
public:
	std::vector<Input*> inputs;
	transfer_function transfFunc;
	float value;
	float neuralConstant;

	Neuron();

	~Neuron();

	//unsigned long timestamp;
	unsigned long RID;

	void update_value(); // recomputes the value of the neuron after input has been updated

	// pushes the output to the targets
	void push_output();

	// retrieves the list of targets
	void retrieve_targets(unsigned long opRID,
			std::vector<Neuron*> &out_targets);

	OutputSocket output; // this socket is connected to other inputs or to the network's main outputs

protected:
	float compute_sum(int count, float input_array[], float weight_array[]);
};

#endif // __neuron_h__
