#include "OutputSocket.h"
#include "InputSocket.h"

#include <boglfw/utils/log.h>

#include <algorithm>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

void OutputSocket::addTarget(InputSocket* pTarget) {
	if (std::find(target_list.begin(), target_list.end(), pTarget) == target_list.end())
		target_list.push_back(pTarget);
}

void OutputSocket::removeTarget(InputSocket* pTarget) {
	auto it = std::find(target_list.begin(), target_list.end(), pTarget);
	if (it == target_list.end()) {
		ERROR("OutputSocket::removeTarget called with argument not in list!");
		return;
	}
	target_list.erase(it);
}

void OutputSocket::push_value(float value) {
	for (auto it : target_list) {
		it->push(value);
	}
#ifdef DEBUG
	cachedValue_ = value;
#endif
}
