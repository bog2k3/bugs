/*
 * AttachPoint.cpp
 *
 *  Created on: Nov 10, 2014
 *      Author: bog
 */

#include "AttachPoint.h"
#include "RigidBody.h"

glm::vec2 AttachPoint::getWorldPos() const {
	return pObject->localToWorld(offset);
}
