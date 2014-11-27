/*
 * World.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "objects/WorldObject.h"
#include "input/operations/IOperationSpatialLocator.h"
#include "input/IWorldManager.h"
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <vector>
#include <list>
#include <deque>

class b2World;

class World : public IOperationSpatialLocator, public IWorldManager, public b2QueryCallback {
public:
	World(b2World* physWld);
	virtual ~World();

	WorldObject* getObjectAtPos(glm::vec2 pos);
	void getObjectsInBox(AlignedBox box, std::vector<WorldObject*> &outVec);

	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	virtual bool ReportFixture(b2Fixture* fixture);

	void setRenderContext(ObjectRenderContext ctxt) { renderContext = ctxt; }
	void draw();

	void addObject(WorldObject* obj);
	void removeObject(WorldObject* obj);

protected:
	b2World* physWld;
	std::list<WorldObject*> objects;
	ObjectRenderContext renderContext;

	std::deque<b2Fixture*> b2QueryResult;
};

#endif /* WORLD_H_ */
