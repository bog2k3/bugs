/*
 * Entity.cpp
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#include "Entity.h"
#include "../World.h"
#include <cassert>

Entity::~Entity() {
	assert(markedForDeletion_ && "You should never call delete on an Entity directly! (use destroy() instead)");
}

void Entity::destroy() {
	markedForDeletion_ = true;
	World::getInstance()->destroyEntity(this);
}
