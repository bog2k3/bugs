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
	float inputBias = 0;

	Neuron();

	~Neuron();

	void setTranferFunction(transferFuncNames fn);

	void update_value(); // recomputes the value of the neuron after input has been updated

	void push_output() { output.push_value(value); }

	// retrieves the list of targets
//	void retrieve_targets(unsigned long opRID, std::vector<Neuron*> &out_targets);

	OutputSocket output; // this socket is connected to other inputs or to the network's main outputs

private:
	float value = 0;
	bool isZeroCmdSignal = false;
	transfer_function transfFunc = transfer_fn_one;
};

#endif // __neuron_h__
