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
	float inputBias = 0;
	float neuralParam = 2;

	Neuron();

	~Neuron();

	void setTranferFunction(transferFuncNames fn);
	void addInput(std::unique_ptr<InputSocket> &&input, float priority);
	void commitInputs();	// sorts all inputs by priority in their final positions and deletes priority data

	void update_value(); // recomputes the value of the neuron after input has been updated
	inline void push_output() { output.push_value(value); }

	OutputSocket output; // this socket is connected to other inputs or to the network's main outputs

private:
	float value = 0;
	bool isZeroCmdSignal = false;
	transfer_function transfFunc = transfer_fn_one;

	std::vector<std::unique_ptr<InputSocket>> inputs;
	std::vector<float> *pInputPriorities = new std::vector<float>();
};

#endif // __neuron_h__
