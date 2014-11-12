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

class ISpatialResolver;
class RigidBody;

class Physics {
public:
	Physics(ISpatialResolver* resolver);
	virtual ~Physics();

	void update(float dt);

private:
	ISpatialResolver* spatialResolver;
	std::vector<RigidBody*> rigidBodies;

	void updateAndApplySpringForces();
	void updateAndApplyAccelerationsAndVelocities(float dt);
	void moveAndCheckCollisions(float dt);
	void applyForceToObject(RigidBody* obj, glm::vec2 localOffset, glm::vec2 force);
};

#endif /* PHYSICS_PHYSICS_H_ */
