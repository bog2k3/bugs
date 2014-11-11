/*
 * AttachPoint.h
 *
 *  Created on: Nov 10, 2014
 *      Author: bog
 */

#ifndef PHYSICS_ATTACHPOINT_H_
#define PHYSICS_ATTACHPOINT_H_

#include <glm/vec2.hpp>

class RigidBody;

class AttachPoint {
public:
	RigidBody* pObject;
	glm::vec2 offset;

	AttachPoint();
};

#endif /* PHYSICS_ATTACHPOINT_H_ */
