/*
* OutputSocket is a multicast socket, connecting one output to multiple other Inputs
*/
#ifndef __OutputSocket_h__
#define __OutputSocket_h__

#include <vector>

class Input;

class OutputSocket {
public:
	void addTarget(Input* pTarget); // connect a new Input to this node
	void push_value(double value); // push the output value to all connected Inputs
	std::vector<Input*>& getTargets(); // retrieves the list of targets

	OutputSocket()
		: target_list()
	{ }

	OutputSocket(const OutputSocket& original)
		: target_list(original.target_list)
	{ }
private:
	std::vector<Input*> target_list; // the list of targets to which the Input is sent to
};

#endif //__OutputSocket_h__
