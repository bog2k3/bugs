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
	/* definitions:
	 * 		miu - friction coefficient
	 * 		alpha - speed-dependency coefficient for friction
	 * 		m - object's mass
	 * 		v - object's linear speed
	 * 		Ff - linear friction force
	 * 		w - object's angular speed
	 * 		tau - torque of rotational friction force
	 * 		I - body's moment of inertia
	 * 		E - total energy of object (translational + rotational)
	 * 		Q - energy lost by object in unit of time because of total friction
	 * 		gamma - ratio between initial and final speed and angular speed
	 *
	 * 	formulas:
	 * 		miu - (miu1 + miu2)/2     "surface dependent on both surfaces in contact"
	 * 		alpha - should somehow be surface dependent, but we'll set it constant for now
	 * 		Ff = miu*m*(1 + alpha*v^2)
	 * 		tau = pi/16*miu*m*(16 + alpha*w^2*(width^2+height^2))		"width and height belong to object in question"
	 * 		E = (m*v^2 + I*w^2) / 2		"total energy = kinetic (linear) energy + rotational energy"
	 * 		Q = (Pf + Pfw) * dt, where Pf is the "power of friction force" and Pfw is the "power of rotational friction force"
	 * 			Pf = the amount of linear kinetic energy deduced in unit of time by the friction force
	 * 			Pfw = amount of rotational kinetic energy deduced in unit of time by the rotational friction
	 *
	 * 			Pf = Ff * v
	 * 			Pfw = tau * w
	 * 		thus:
	 * 			Q = miu*m*((1+alpha*v^2)*v + pi/16*(16+alpha*w^2*(width^2+height^2)*w)) * dt
	 * 		gamma^2 = 1 - Q/E
	 *
	 * 	finally:
	 * 		if gamma <= 0 then:
	 * 			v <- 0
	 * 			w <- 0
	 * 		else:
	 * 			v <- gamma * v
	 * 			w <- gamma * w
	 */
	float miu = 0.2f; // should be surface-dependent
	float alpha = 0.1f; // speed-dependency coefficient
	float m = obj->mass;
	float v = length(obj->velocity);
	float w = obj->angularVelocity;
	float I = obj->getMomentOfInertia();
	float E = 0.5f * (m * sqr(v) + I * sqr(w));
	if (E == 0)
		return;
	glm::vec2 objSize = obj->getLocalBoundingBox().getSize();

	float Ff = miu * m * (1 + alpha * sqr(v));
	float tau = PI/16 * miu * m * (16 + alpha*sqr(w)*(sqr(objSize.x) + sqr(objSize.y)));
	float Q = (Ff * v + tau * abs(w)) * dt;
	float gamma = Q > E ? 0 : sqrt(1 - Q/E);
	obj->velocity *= gamma;
	obj->angularVelocity *= gamma;
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
