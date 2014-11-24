/*
 * RigidBody.h
 *
 *  Created on: Nov 10, 2014
 *      Author: bog
 */

#ifndef PHYSICS_RIGIDBODY_H_
#define PHYSICS_RIGIDBODY_H_

#include <glm/vec2.hpp>
#include <glm/mat3x2.hpp>
#include "../math/math2D.h"

class RigidBody {
public:
	RigidBody(float mass, glm::vec2 position, float rotation, glm::vec2 initialVelocity, float initialAngularVelocity);
	virtual ~RigidBody() {}

	// returns the smallest world-aligned bounding box completely containing the object
	virtual AlignedBox getAlignedBoundingBox() const = 0;

	// returns the smallest object-aligned bounding box completely containing the object
	virtual ArbitraryBox getOrientedBoundingBox() const = 0;

	// returns the smallest bounding box in object's space
	virtual AlignedBox getLocalBoundingBox() const = 0;

	// return the moment of inertia for the body
	virtual float getMomentOfInertia() const = 0;

	// returns the mass of the body
	float getMass() const { return mass; }

	// returns the world position of the object's center of weight
	glm::vec2 getPosition() const { return position; }

	// returns the world velocity of the object
	glm::vec2 getVelocity() const { return velocity; }

	// returns the rotation of the object (around center of weight)
	float getRotation() const { return rotation; }

	// returns the angular velocity of the object (rotation happens around center of weight)
	float getAngularVelocity() const { return angularVelocity; }

	glm::vec2 localToWorld(glm::vec2 local) const;
	glm::vec2 worldToLocal(glm::vec2 wld) const;

	void teleport(glm::vec2 where);

	bool isFixed;

private:
	friend class Physics;
	float mass;
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec2 prevVelocity;
	glm::vec2 acceleration;
	float rotation;
	float angularVelocity;
	float prevAngularVelocity;
	glm::vec2 resultantForce;
	float resultantTorque;
	glm::mat3x2 matLocalToWorld;
	void updateMatrix();
};

#endif /* PHYSICS_RIGIDBODY_H_ */
