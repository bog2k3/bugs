/*
 * Physics.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#ifndef PHYSICS_PHYSICS_H_
#define PHYSICS_PHYSICS_H_

#include <vector>
#include <glm/vec2.hpp>

class IPhysicsSpatialResolver;
class RigidBody;

class Physics {
public:
	Physics(IPhysicsSpatialResolver* resolver);
	virtual ~Physics();

	void update(float dt, bool computeEnergy=false);

	float getTranslationalEnergy() { return frameTranslationalEnergy; }
	float getRotationalEnergy() { return frameRotationalEnergy; }
	float getElasticPotentialEnergy() { return frameElasticPotentialEnergy; }

private:
	IPhysicsSpatialResolver* spatialResolver;
	std::vector<RigidBody*> rigidBodies;

	float frameTranslationalEnergy;
	float frameRotationalEnergy;
	float frameElasticPotentialEnergy;

	void updateAndApplySpringForces(float dt, bool computeEnergy);
	void updateAndApplyAccelerationsAndVelocities(float dt, bool computeEnergy);
	void moveAndCheckCollisions(float dt);
	void applyForceToObject(RigidBody* obj, glm::vec2 localOffset, glm::vec2 force);
	void applyFriction(RigidBody* obj, float dt);
};

#endif /* PHYSICS_PHYSICS_H_ */
