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
	const RigidBody* pObject;
	const glm::vec2 offset;

	AttachPoint(RigidBody* pObject, glm::vec2 offset)
		: pObject(pObject), offset(offset) {
	}

	glm::vec2 getWorldPos() const;
};

#endif /* PHYSICS_ATTACHPOINT_H_ */
