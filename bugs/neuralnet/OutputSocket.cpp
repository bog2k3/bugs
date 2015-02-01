#include "OutputSocket.h"
#include <algorithm>
#include "InputSocket.h"

void OutputSocket::addTarget(InputSocket* pTarget) {
	if (std::find(target_list.begin(), target_list.end(), pTarget) == target_list.end())
		target_list.push_back(pTarget);
}

std::vector<InputSocket*>& OutputSocket::getTargets() {
	return target_list;
}

void OutputSocket::push_value(float value) {
	for (auto it : target_list) {
		it->push(value);
	}
}
