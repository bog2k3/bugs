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

Entity::~Entity() {
	assertDbg(markedForDeletion_ && "You should never call delete on an Entity directly! (use destroy() instead)");
}

void Entity::destroy() {
	if (markedForDeletion_) {
		LOGLN("WARNING: destroy called more than once!");
		return;
	}
	markedForDeletion_ = true;
	World::getInstance()->destroyEntity(this);
}

void Entity::serialize(BinaryStream &stream) { assertDbg(false && "forgot to override this?"); }
SerializationObjectTypes Entity::getSerializationType() { assertDbg(false && "forgot to override this?"); }
