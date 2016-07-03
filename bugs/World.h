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
#include "utils/MTVector.h"
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <vector>
#include <memory>

class b2World;
class b2Body;
struct b2AABB;
class PhysDestroyListener;

class World : public IOperationSpatialLocator {
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
	MTVector<Entity*> entsToDestroy;
	MTVector<std::unique_ptr<Entity>> entsToTakeOver;
	PhysDestroyListener *destroyListener_ = nullptr;

	void destroyPending();
	void takeOverPending();

	std::vector<b2Fixture*> getFixtures(b2AABB const& aabb);
	bool testEntity(Entity &e, EntityType filterTypes, Entity::FunctionalityFlags filterFlags);
};

#endif /* WORLD_H_ */

