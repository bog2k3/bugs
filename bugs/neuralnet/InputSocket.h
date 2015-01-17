/*
 * InputSocket.h
 *
 *  Created on: Jan 17, 2015
 *      Author: bog
 */

#ifndef NEURALNET_INPUTSOCKET_H_
#define NEURALNET_INPUTSOCKET_H_

class Neuron;

class InputSocket {
public:
	float value;
	float weight;
	Neuron* pParentNeuron;

	InputSocket(Neuron* pParentNeuron, float weight) :
		value(0), weight(weight), pParentNeuron(pParentNeuron) {
	}

	// pushes a new value into the input
	void push(float value) {
		this->value = value;
	}
};

#endif /* NEURALNET_INPUTSOCKET_H_ */
