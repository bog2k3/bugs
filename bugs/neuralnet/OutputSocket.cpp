#include "Neuron.h"
#include "OutputSocket.h"

#include <algorithm>

void OutputSocket::addTarget(Input* pTarget) {
	if (std::find(target_list.begin(), target_list.end(), pTarget) == target_list.end())
		target_list.push_back(pTarget);
}

std::vector<Input*>& OutputSocket::getTargets() {
	return target_list;
}

void OutputSocket::push_value(double value) {
	std::vector<Input*>::iterator it = target_list.begin(),
		itE = target_list.end();
	for (; it != itE; ++it) {
		Input* pTargetInput = *it;
		pTargetInput->push(value);
	}
}
