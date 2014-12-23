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
#include "renderOpenGL/RenderContext.h"
#include "drawable.h"
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <vector>
#include <deque>

class b2World;
class b2Body;

class World : public IOperationSpatialLocator, public IWorldManager, public b2QueryCallback {
public:
	static World* getInstance();
	virtual ~World();

	WorldObject* getObjectAtPos(glm::vec2 pos);
	void getObjectsInBox(AlignedBox box, std::vector<WorldObject*> &outVec);

	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	virtual bool ReportFixture(b2Fixture* fixture);

	void update(float dt);

	void addObject(WorldObject* obj);
	void removeObject(WorldObject* obj);

	void setPhysics(b2World* physWld);
	b2World* getPhysics() { return physWld; }
	b2Body* getGroundBody() { return groundBody; }

protected:
	World();
	b2World* physWld;
	b2Body* groundBody;
	std::vector<WorldObject*> objects;

	std::deque<b2Fixture*> b2QueryResult;

	friend void draw<World*>(World*& wld, RenderContext& ctx);
};

template<> void draw(World*& wld, RenderContext& ctx);

#endif /* WORLD_H_ */
