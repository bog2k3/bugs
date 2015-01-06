/*
 * UpdateList.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: bog
 */

#include "UpdateList.h"

template <> void update(UpdateList &l, float dt) {
	l.update(dt);
}
template <> void update(UpdateList* &l, float dt) {
	l->update(dt);
}
