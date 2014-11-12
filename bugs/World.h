/*
 * World.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "physics/ISpatialResolver.h"
#include "objects/IWorldObject.h"
#include <vector>

class World : public ISpatialResolver {
public:
	World();
	virtual ~World();

	void retrieveObjectsInCircle(Circle const &circle, std::vector<RigidBody*> &outVector);
	void retrieveObjects(std::vector<RigidBody*> &outVector);
	void retrieveObjects(std::vector<Spring*> &outVector);

	void setRenderContext(ObjectRenderContext ctxt) { renderContext = ctxt; }
	void update(float dt);
	void draw();

	void addObject(IWorldObject* obj);

protected:
	std::vector<IWorldObject*> objects;
	ObjectRenderContext renderContext;
};

#endif /* WORLD_H_ */
