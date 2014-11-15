/*
 * World.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "objects/WorldObject.h"
#include "physics/IPhysicsSpatialResolver.h"
#include "input/operations/IOperationSpatialLocator.h"
#include "input/IWorldManager.h"
#include <vector>
#include <list>

class World : public IPhysicsSpatialResolver, public IOperationSpatialLocator, public IWorldManager {
public:
	World();
	virtual ~World();

	void retrieveObjectsInCircle(Circle const &circle, std::vector<RigidBody*> &outVector);
	void retrieveObjects(std::vector<RigidBody*> &outVector);
	void retrieveObjects(std::vector<Spring*> &outVector);

	WorldObject* getObjectAtPos(glm::vec2 pos);
	void getObjectsInBox(AlignedBox box, std::vector<WorldObject*> &outVec);

	void setRenderContext(ObjectRenderContext ctxt) { renderContext = ctxt; }
	void updatePrePhysics(float dt);
	void updatePostPhysics(float dt);
	void draw();

	void addObject(WorldObject* obj);
	void removeObject(WorldObject* obj);

protected:
	std::list<WorldObject*> objects;
	std::vector<RigidBody*> rigidBodiesCache;
	std::vector<Spring*> springsCache;
	ObjectRenderContext renderContext;
};

#endif /* WORLD_H_ */
