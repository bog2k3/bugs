/*
* OutputSocket is a multicast socket, connecting one output to multiple other Inputs
*/
#ifndef __OutputSocket_h__
#define __OutputSocket_h__

#include <vector>

class InputSocket;

class OutputSocket {
public:
	void addTarget(InputSocket* pTarget); // connect a new Input to this node
	void push_value(float value); // push the output value to all connected Inputs
	std::vector<InputSocket*>& getTargets(); // retrieves the list of targets

	OutputSocket()
		: target_list()
	{ }
private:
	std::vector<InputSocket*> target_list; // the list of targets to which the Input is sent to
};

#endif //__OutputSocket_h__
