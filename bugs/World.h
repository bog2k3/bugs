/*
 * World.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "entities/Entity.h"
#include "input/operations/IOperationSpatialLocator.h"
#include "renderOpenGL/RenderContext.h"
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <vector>
#include <memory>

class b2World;
class b2Body;
struct b2AABB;
class PhysDestroyListener;

class World : public IOperationSpatialLocator, public b2QueryCallback {
public:
	static World* getInstance();
	World();
	virtual ~World();

	/**
	 * delete all entities and reset state
	 */
	void reset();

	b2Body* getBodyAtPos(glm::vec2 const& pos) override;
	void getBodiesInArea(glm::vec2 const& pos, float radius, bool clipToCircle, std::vector<b2Body*> &outBodies);

	/// b2QueryCallback::
	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	bool ReportFixture(b2Fixture* fixture) override;

	void setPhysics(b2World* physWld);
	void setDestroyListener(PhysDestroyListener *listener) { destroyListener_ = listener; }
	PhysDestroyListener* getDestroyListener() { return destroyListener_; }
	b2World* getPhysics() { return physWld; }
	b2Body* getGroundBody() { return groundBody; }

	void takeOwnershipOf(std::unique_ptr<Entity> &&e);
	void destroyEntity(Entity* e);

	// returns a vector of all entities that match ALL of the requested features
	std::vector<Entity*> getEntities(EntityType filterTypes, Entity::FunctionalityFlags filterFlags = Entity::FunctionalityFlags::NONE);

	// we have physBody->getEntity(), so:
	std::vector<Entity*> getEntitiesInBox(EntityType filterTypes, Entity::FunctionalityFlags filterFlags, glm::vec2 pos, float radius, bool clipToCircle);

	void update(float dt);
	void draw(RenderContext const& ctx);

protected:
	b2World* physWld;
	b2Body* groundBody;
	std::vector<std::unique_ptr<Entity>> entities;
	std::vector<Entity*> entsToUpdate;
	std::vector<Entity*> entsToDraw;
	std::vector<Entity*> entsToDestroy;
	std::vector<std::unique_ptr<Entity>> entsToTakeOver;
	std::vector<b2Fixture*> b2QueryResult;
	PhysDestroyListener *destroyListener_ = nullptr;

	void destroyPending();
	void takeOverPending();

	// std::vector<Entity*> getEntities(std::function<bool(Entity const&)> predicate);
};

#endif /* WORLD_H_ */

