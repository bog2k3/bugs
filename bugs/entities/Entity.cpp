/*
 * Entity.cpp
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#include "Entity.h"
#include "../World.h"
#include "../utils/assert.h"
#include "../utils/log.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

Entity::~Entity() {
	assertDbg((!managed_ || markedForDeletion_) && "You should never call delete on a managed Entity directly! (use destroy() instead)");
}

void Entity::destroy() {
	if (markedForDeletion_) {
		return;
	}
	markedForDeletion_ = true;
	if (managed_)
		World::getInstance()->destroyEntity(this);
	else
		delete this;
}

void Entity::serialize(BinaryStream &stream) { assertDbg(false && "forgot to override this?"); }

SerializationObjectTypes Entity::getSerializationType() {
	assertDbg(false && "forgot to override this?");
	return (SerializationObjectTypes)0;
}

