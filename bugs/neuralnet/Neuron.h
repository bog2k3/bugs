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
	float neuralParam = 2;

	Neuron();

	~Neuron();

	void setTranferFunction(transferFuncNames fn);

	void update_value(); // recomputes the value of the neuron after input has been updated

	inline void push_output() { output.push_value(value); }

	OutputSocket output; // this socket is connected to other inputs or to the network's main outputs

private:
	float value = 0;
	bool isZeroCmdSignal = false;
	transfer_function transfFunc = transfer_fn_one;
};

#endif // __neuron_h__
