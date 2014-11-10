/*
 * RigidBody.h
 *
 *  Created on: Nov 10, 2014
 *      Author: bog
 */

#ifndef PHYSICS_RIGIDBODY_H_
#define PHYSICS_RIGIDBODY_H_

#include "../math/math.h"
#include <glm/vec2.hpp>

class RigidBody {
public:
	virtual ~RigidBody() {}

	// returns the smallest world-aligned bounding box completely containing the object
	virtual AlignedBox getAlignedBoundingBox() = 0;

	// returns the smallest object-aligned bounding box completely containing the object
	virtual ArbitraryBox getOrientedBoundingBox() = 0;

	// returns the world position of the object's center of weight
	virtual glm::vec2 getPosition() = 0;

	// returns the world velocity of the object
	virtual glm::vec2 getVelocity() = 0;

	// returns the rotation of the object (around center of weight)
	virtual float getRotation() = 0;

	// returns the angular velocity of the object (rotation happens around center of weight)
	virtual float getAngularVelocity() = 0;
};

#endif /* PHYSICS_RIGIDBODY_H_ */
