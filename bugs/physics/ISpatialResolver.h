#pragma once

#include <vector>

struct Circle;
class PhysicsObject;

class ISpatialResolver
{
public:
	virtual ~ISpatialResolver(){}

	// must return the objects that are partially or completely inside the specified circle
	// the objects should be appended to the vector, without interfering with its current contents.
	// the callee must make sure it does not return the same object multiple times.
	virtual void retrieveObjectsInCircle(Circle const &circle, std::vector<PhysicsObject*> &outVector)=0;
};
