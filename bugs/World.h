/*
 * World.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "input/operations/IOperationSpatialLocator.h"
#include "renderOpenGL/RenderContext.h"
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <vector>
#include <deque>

class b2World;
class b2Body;
class Entity;

class World : public IOperationSpatialLocator, public b2QueryCallback {
public:
	static World* getInstance();
	virtual ~World();

	b2Body* getBodyAtPos(glm::vec2 pos) override;

	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	bool ReportFixture(b2Fixture* fixture) override;

	void setPhysics(b2World* physWld);
	b2World* getPhysics() { return physWld; }
	b2Body* getGroundBody() { return groundBody; }

	void takeOwnershipOf(Entity* e);

protected:
	World();
	b2World* physWld;
	b2Body* groundBody;
	std::vector<Entity*> entities;
	std::deque<b2Fixture*> b2QueryResult;
};

#endif /* WORLD_H_ */
