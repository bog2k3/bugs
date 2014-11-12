/*
 * RigidBody.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "RigidBody.h"

glm::vec2 RigidBody::localToWorld(glm::vec2 local) const {
	return matLocalToWorld * glm::vec3(local, 1);
}
