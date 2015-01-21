/*
 * UpdateList.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: bog
 */

#include "UpdateList.h"
#include "Event.h"

void UpdateList::add(updatable_wrap &&w) {
	list_.push_back(w);
}

void UpdateList::update(float dt) {
	for (auto &w : list_)
			w.update(dt);
}
