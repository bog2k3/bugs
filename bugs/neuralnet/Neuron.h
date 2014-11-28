/*
 *	neuron
 */

#ifndef __neuron_h__
#define __neuron_h__

#include <vector>
#include "functions.h"
#include "OutputSocket.h"

class Neuron;

class Input {
public:
	double value;
	double weight;
	Neuron* pParentNeuron;

	Input(Neuron* pParentNeuron) :
		value(0), weight(0), pParentNeuron(pParentNeuron) {
	}

	void push(double value); // pushes a new value into the input
};

class Neuron {
public:
	unsigned int ID;
	std::vector<Input*> inputs;
	transfer_function transfFunc;
	double value;
	double bias;
	double transferConstant;

	bool isInput;
	bool isOutput;

	Neuron(int ID);

	Neuron(const Neuron& original);

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
	double compute_sum(int count, double input_array[], double weight_array[]);
};

#endif // __neuron_h__
