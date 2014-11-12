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
#include <glm/mat3x2.hpp>

class RigidBody {
public:
	virtual ~RigidBody() {}

	// returns the smallest world-aligned bounding box completely containing the object
	virtual AlignedBox getAlignedBoundingBox() const = 0;

	// returns the smallest object-aligned bounding box completely containing the object
	virtual ArbitraryBox getOrientedBoundingBox() const = 0;

	// returns the world position of the object's center of weight
	glm::vec2 getPosition() const { return position; }

	// returns the world velocity of the object
	glm::vec2 getVelocity() const { return velocity; }

	// returns the rotation of the object (around center of weight)
	float getRotation() const { return rotation; }

	// returns the angular velocity of the object (rotation happens around center of weight)
	float getAngularVelocity() const { return angularVelocity; }

	glm::vec2 localToWorld(glm::vec2 local) const;

private:
	friend class PhysicsEngine;
	glm::vec2 position;
	glm::vec2 velocity;
	float rotation;
	float angularVelocity;
	glm::vec2 resultantForce;
	float resultantAngularMomentum;

	glm::mat3x2 matLocalToWorld;
	void updateMatrix(bool rotation, bool translation);
};

#endif /* PHYSICS_RIGIDBODY_H_ */
