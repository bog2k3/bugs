/*
 * UpdateList.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef UPDATELIST_H_
#define UPDATELIST_H_

#include "updatable.h"
#include "log.h"
#include <vector>
#include <algorithm>
#include <map>

class UpdateList {
public:
	UpdateList() = default;

	void add(updatable_wrap &&w);
	void update(float dt);

private:
	std::vector<updatable_wrap> list_;
};

#endif /* UPDATELIST_H_ */
