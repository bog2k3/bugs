/*
 * World.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "physics/ISpatialResolver.h"
#include <vector>
#include <list>
#include "objects/WorldObject.h"

class World : public ISpatialResolver {
public:
	World();
	virtual ~World();

	void retrieveObjectsInCircle(Circle const &circle, std::vector<RigidBody*> &outVector);
	void retrieveObjects(std::vector<RigidBody*> &outVector);
	void retrieveObjects(std::vector<Spring*> &outVector);

	void setRenderContext(ObjectRenderContext ctxt) { renderContext = ctxt; }
	void updatePrePhysics(float dt);
	void updatePostPhysics(float dt);
	void draw();

	void addObject(WorldObject* obj);

protected:
	std::list<WorldObject*> objects;
	std::vector<RigidBody*> rigidBodiesCache;
	std::vector<Spring*> springsCache;
	ObjectRenderContext renderContext;
};

#endif /* WORLD_H_ */
