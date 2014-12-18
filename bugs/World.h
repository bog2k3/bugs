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
#include "updatable.h"
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

	void setRenderContext(ObjectRenderContext ctxt) { renderContext = ctxt; }
	void draw();
	void update(float dt);

	void addObject(WorldObject* obj);
	void removeObject(WorldObject* obj);

	void addUpdatable(updatable_wrap w);
	void removeUpdatable(updatable_wrap w) {
		updatables.erase(std::remove_if(updatables.begin(), updatables.end(), [&w] (const updatable_wrap& x) {
			return x.equal_value(w);
		}), updatables.end());
	}

	void setPhysics(b2World* physWld);
	b2World* getPhysics() { return physWld; }
	b2Body* getGroundBody() { return groundBody; }

protected:
	World();
	b2World* physWld;
	b2Body* groundBody;
	std::vector<WorldObject*> objects;
	std::vector<updatable_wrap> updatables;
	ObjectRenderContext renderContext;

	std::deque<b2Fixture*> b2QueryResult;
};

#endif /* WORLD_H_ */
