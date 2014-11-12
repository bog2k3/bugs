/*
 * Physics.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Physics.h"
#include "ISpatialResolver.h"
#include "RigidBody.h"
#include "Spring.h"
#include <glm/gtx/rotate_vector.hpp>
#include "../math/math.h"

using namespace glm;

Physics::Physics(ISpatialResolver* resolver)
	: spatialResolver(resolver)
{
}

Physics::~Physics() {
	// TODO Auto-generated destructor stub
}

void Physics::update(float dt) {
	// step 1:
	// compute forces in all springs and accumulate these forces to the rigid bodies they're attached to
	updateAndApplySpringForces();

	rigidBodies.clear();
	spatialResolver->retrieveObjects(rigidBodies);

	// step 2:
	// Apply forces, compute accelerations, apply accelerations, update velocities (linear and angular)
	updateAndApplyAccelerationsAndVelocities(dt);

	// step 3:
	// apply velocities, move objects and check for collisions
	moveAndCheckCollisions(dt);
}

void Physics::updateAndApplySpringForces() {
	std::vector<Spring*> springs;
	spatialResolver->retrieveObjects(springs);
	for (Spring* s : springs) {
		vec2 force = s->getForce();
		if (force.x == 0 && force.y == 0)
			continue;
		// apply force to s' first attachment point as it is, and to the second reversed.
		applyForceToObject(s->a1.pObject, s->a1.offset, force);
		applyForceToObject(s->a2.pObject, s->a2.offset, -force);
	}
}

void Physics::updateAndApplyAccelerationsAndVelocities(float dt) {
	for (RigidBody* body : rigidBodies) {
		vec2 acceleration = body->resultantForce / body->mass;
		body->velocity += acceleration * dt;
		float angularAcceleration = body->resultantTorque / body->getMomentOfInertia();
		body->angularVelocity += angularAcceleration * dt;

		// clear accumulation variables:
		body->resultantForce = vec2(0);
		body->resultantTorque = 0;
	}
}

void Physics::moveAndCheckCollisions(float dt) {
	for (RigidBody* body : rigidBodies) {
		body->position += body->velocity * dt;
		body->rotation += body->angularVelocity * dt;
	}
}

void Physics::applyForceToObject(RigidBody* obj, vec2 localOffset, vec2 force) {
	// the force is split into a tangent and normal vectors by the axis from the force's application point
	// to the receiver's center of weight.
	// the tangent vector will manifest as an angular momentum;
	// the normal vector will manifest as a linear acceleration

	// localOffset is in obj's local space
	// force is in world space

	// get the normal axis in world space
	vec2 normalAxis = rotate(normalize(localOffset), obj->getRotation());
	vec2 normalForce = normalAxis * dot(normalAxis, force);
	vec2 tangentForce = force - normalForce;
	float tangentForceSign = cross2D(normalAxis, tangentForce) > 0 ? +1 : -1;

	obj->resultantForce += normalForce;
	obj->resultantTorque += tangentForce.length() * tangentForceSign * localOffset.length();
}
