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
	void removeTarget(InputSocket* pTarget);
	void push_value(float value); // push the output value to all connected Inputs
	std::vector<InputSocket*> const& getTargets() { return target_list; } // retrieves the list of targets

	OutputSocket()
		: target_list()
	{ }

#ifdef DEBUG
	float debugGetCachedValue() { return cachedValue_; }
#endif

private:
	std::vector<InputSocket*> target_list; // the list of targets to which the Input is sent to

#ifdef DEBUG
	float cachedValue_ = 0;
#endif
};

#endif //__OutputSocket_h__
