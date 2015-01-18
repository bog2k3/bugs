#ifndef __traverser_h__
#define __traverser_h__

#include <vector>

class NeuralNet;
class Neuron;

class Traverser {
public:
	Traverser(NeuralNet* pNet);
	~Traverser();

	// reset internal state for a new round of full traversal
	void reset();

	// returns the next 'layer' of neurons iteratively. Neurons are not really organized into layers,
	// but the Traverser returns at each step the next neurons that have been touched by navigating
	// from the NeuralNet's inputs to all connected neurons, then to all neurons connected to the last
	// neurons and so on, until all neurons are visited.
	// the NeuralNet has been completely traversed when this function returns an empty vector.
	void getNextLayer(std::vector<Neuron*> &out);

	// after finishing with getNextLayer, this function returns isolated neurons (if any exist).
	// some neurons may not be connected to any other neurons (they have no inputs) so, they are isolated
	// Don't call this method before the traversal is done, or it will return an empty vector
	std::vector<Neuron*> getIsolatedNeurons();

private:

	unsigned int RID;
	NeuralNet* pNetwork;
	std::vector<Neuron*> crt_neurons;
	bool finished;
};

#endif //__traverser_h__
