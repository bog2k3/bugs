/*
 * Physics.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#include "Physics.h"
#include "RigidBody.h"
#include "Spring.h"
#include <glm/gtx/rotate_vector.hpp>
#include "../math/math2D.h"
#include "IPhysicsSpatialResolver.h"

using namespace glm;

Physics::Physics(IPhysicsSpatialResolver* resolver)
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
		if (body->isFixed) {
			body->velocity = vec2(0);
			body->angularVelocity = 0;
			continue;
		}
		vec2 acceleration = body->resultantForce / body->mass;
		body->velocity += acceleration * dt;
		float angularAcceleration = body->resultantTorque / body->getMomentOfInertia();
		body->angularVelocity += angularAcceleration * dt;

		// clear accumulation variables:
		body->resultantForce = vec2(0);
		body->resultantTorque = 0;

		// apply friction:
		applyFriction(body, dt);
	}
}

void Physics::applyFriction(RigidBody* obj, float dt) {
	// 1. linear friction
	// Ff = fCoeff * obj->getMass() * (1 + speedCoeff * sqr(speed))
	// af = Ff / mass
	float miu = 0.2f; // miu, should be surface-dependent
	float speedCoeff = 0.1f; // how much speed counts
	float speed = length(obj->getVelocity());

	float frictionAccel = miu * (1 + speedCoeff * sqr(speed)) * dt;
	if (frictionAccel > speed)
		obj->velocity = vec2(0);
	else
		obj->velocity *= 1 - frictionAccel / speed;

	// 2. angular friction
	// dFfw = u*p*(1+a*w^2*r^2) dr dphi (approximate shape by an ellipse, where R(phi) = sqrt(1/(cos^2(phi)/a^2 + sin^2(phi)/b^2))
	// tauF = integral on obj's surface of dFfw * r, r < [0, R(phi)], phi < [0, 2*pi]
	// tauF = pi/16 * u * m * (16 + alpha * w^2 ( width^2 + height^2))
	// wf = tauF / I
	float angCoeff = 0.12f; // (alpha) how much rotation counts
	float w = obj->angularVelocity;
	glm::vec2 objSize = obj->getLocalBoundingBox().getSize();
	float frictionTorque /*tau*/ = miu * obj->mass * PI/16 * (16 + angCoeff * sqr(w)*(sqr(objSize.x)+sqr(objSize.y)));
	float wf = frictionTorque / obj->getMomentOfInertia() * dt;
	if (wf > abs(obj->angularVelocity))
		obj->angularVelocity = 0;
	else
		obj->angularVelocity -= wf * sign(obj->angularVelocity);
}

void Physics::moveAndCheckCollisions(float dt) {
	for (RigidBody* body : rigidBodies) {
		body->position += body->velocity * dt;
		body->rotation += body->angularVelocity * dt;
		body->updateMatrix();
	}
}

void Physics::applyForceToObject(RigidBody* obj, vec2 localOffset, vec2 force) {
	// the force is split into a tangent and normal vectors by the axis from the force's application point
	// to the receiver's center of weight.
	// the tangent vector will manifest as an angular momentum;
	// the normal vector will manifest as a linear acceleration

	// localOffset is in obj's local space
	// force is in world space

	if (eqEps(localOffset.x, 0) && eqEps(localOffset.y, 0)) {
		// force acting on the center of weight
		obj->resultantForce += force;
	} else {
		// eccentric force; get the normal axis in world space
		vec2 normalAxis = rotate(normalize(localOffset), obj->rotation);
		vec2 normalForce = normalAxis * dot(normalAxis, force);
		vec2 tangentForce = force - normalForce;
		float tangentForceSign = cross2D(normalAxis, tangentForce) > 0 ? +1 : -1;

		obj->resultantForce += normalForce;
		obj->resultantTorque += length(tangentForce) * tangentForceSign * length(localOffset);
	}
}
