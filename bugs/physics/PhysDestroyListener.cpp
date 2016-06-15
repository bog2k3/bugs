/*
 * PhysDestroyListener.cpp
 *
 *  Created on: Mar 22, 2015
 *      Author: bog
 */

#include "PhysDestroyListener.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

void PhysDestroyListener::SayGoodbye(b2Joint* joint) {
	auto &vec = mapCallbacks[joint];
	for (auto &cb : vec)
		if (cb)
			cb(joint);
	vec.clear();
}

unsigned PhysDestroyListener::addCallback(b2Joint* joint, callbackType cb) {
	auto &vec = mapCallbacks[joint];
	vec.push_back(cb);
	return vec.size();
}

void PhysDestroyListener::removeCallback(b2Joint* joint, unsigned handle) {
	auto &vec = mapCallbacks[joint];
	if (handle <= vec.size())
		vec[handle-1] = nullptr;
}
