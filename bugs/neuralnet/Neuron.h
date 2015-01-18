/*
 *	neuron
 */

#ifndef __neuron_h__
#define __neuron_h__

#include "functions.h"
#include "OutputSocket.h"
#include <vector>
#include <memory>

class Neuron;
class InputSocket;

class Neuron {
public:
	std::vector<std::unique_ptr<InputSocket>> inputs;
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
	void retrieve_targets(unsigned long opRID, std::vector<Neuron*> &out_targets);

	OutputSocket output; // this socket is connected to other inputs or to the network's main outputs
};

#endif // __neuron_h__
