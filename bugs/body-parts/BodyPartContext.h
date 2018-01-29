/*
 * BodyPartContext.h
 *
 *  Created on: Dec 7, 2017
 *      Author: bog
 */

#ifndef BODY_PARTS_BODYPARTCONTEXT_H_
#define BODY_PARTS_BODYPARTCONTEXT_H_

class Bug;
class UpdateList;

struct BodyPartContext {
	Bug& owner;
	UpdateList& updateList;

	BodyPartContext(Bug& owner, UpdateList& updateList)
		: owner(owner), updateList(updateList) {
	}
};

#endif /* BODY_PARTS_BODYPARTCONTEXT_H_ */
