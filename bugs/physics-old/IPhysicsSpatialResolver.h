#pragma once

#include <vector>

class Circle;
class RigidBody;
class Spring;

class IPhysicsSpatialResolver {
public:
	virtual ~IPhysicsSpatialResolver(){}

	// must return the objects that are partially or completely inside the specified circle
	// the objects should be appended to the vector, without interfering with its current contents.
	// the callee must make sure it does not return the same object multiple times.
	virtual void retrieveObjectsInCircle(Circle const &circle, std::vector<RigidBody*> &outVector)=0;

	// must return all RigidBody type objects in the world
	virtual void retrieveObjects(std::vector<RigidBody*> &outVector)=0;

	// must return all Spring type objects in the world
	virtual void retrieveObjects(std::vector<Spring*> &outVector)=0;
};
