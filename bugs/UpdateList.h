/*
 * UpdateList.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef UPDATELIST_H_
#define UPDATELIST_H_

#include "updatable.h"
#include <vector>
#include <algorithm>

class UpdateList {
public:
	UpdateList() {}
	~UpdateList() {}

	void add(updatable_wrap &&w);
	void remove(updatable_wrap w);
	void update(float dt);

private:
	std::vector<updatable_wrap> list_;
};

template <> void update(UpdateList* l, float dt);

#endif /* UPDATELIST_H_ */
