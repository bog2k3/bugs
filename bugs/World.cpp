/*
 * World.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#include "World.h"
#include "entities/Entity.h"
#include "physics/PhysicsBody.h"
#include "math/math2D.h"
#include "math/box2glm.h"
#include "utils/log.h"
#include "utils/assert.h"
#include "utils/bitFlags.h"
#include "utils/parallel.h"
#include "Infrastructure.h"
#include <glm/glm.hpp>
#include <Box2D/Box2D.h>
#include <algorithm>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

#define MT_UPDATE	// enables parallel update on entities, using the thread pool

static World *instance = nullptr;

World::World() : physWld(nullptr), groundBody(nullptr) {
	assert(instance == nullptr && "attempting to initialize multiple instances of World!!!");
	instance = this;
}

void World::setPhysics(b2World* phys) {
	physWld = phys;
	b2BodyDef gdef;
	gdef.type = b2_staticBody;
	groundBody = physWld->CreateBody(&gdef);
}

World* World::getInstance() {
	assert(instance && "No existing World instance!!!");
	return instance;
}

World::~World() {
	reset();
}

void World::reset() {
	for (auto &e : entities) {
		e->markedForDeletion_= true;
		e.reset();
	}
	for (auto &e : entsToTakeOver) {
		e->markedForDeletion_ = true;
		e.reset();
	}
	entities.clear();
	entsToTakeOver.clear();
	entsToDestroy.clear();
	entsToDraw.clear();
	entsToUpdate.clear();
}

std::vector<b2Fixture*> World::getFixtures(const b2AABB& aabb) {
	class cbWrap : public b2QueryCallback {
	public:
		/// b2QueryCallback::
		/// Called for each fixture found in the query AABB.
		/// @return false to terminate the query.
		bool ReportFixture(b2Fixture* fixture) override {
			fixtures_.push_back(fixture);
			return true;
		}

		std::vector<b2Fixture*> fixtures_;
	} wrap;
	physWld->QueryAABB(&wrap, aabb);
	return std::move(wrap.fixtures_);
}

b2Body* World::getBodyAtPos(glm::vec2 const& pos) {
	b2AABB aabb;
	aabb.lowerBound = g2b(pos) - b2Vec2(0.005f, 0.005f);
	aabb.upperBound = g2b(pos) + b2Vec2(0.005f, 0.005f);
	auto b2QueryResult = getFixtures(aabb);
	if (b2QueryResult.empty())
		return nullptr;
	b2Body* ret = nullptr;
	for (b2Fixture* f : b2QueryResult) {
		if (f->TestPoint(g2b(pos))) {
			ret = f->GetBody();
			break;
		}
	}
	return ret;
}

void World::getBodiesInArea(glm::vec2 const& pos, float radius, bool clipToCircle, std::vector<b2Body*> &outBodies) {
	b2AABB aabb;
	aabb.lowerBound = g2b(pos) - b2Vec2(radius, radius);
	aabb.upperBound = g2b(pos) + b2Vec2(radius, radius);
	auto b2QueryResult = getFixtures(aabb);
	for (b2Fixture* f : b2QueryResult) {
		if (clipToCircle)
			if (glm::length(b2g(f->GetAABB(0).GetCenter()) - pos) > radius)
				continue;
		outBodies.push_back(f->GetBody());
	}
}

void World::takeOwnershipOf(std::unique_ptr<Entity> &&e) {
	assertDbg(e != nullptr);
	entsToTakeOver.push_back(std::move(e));
#warning make Thread Safe!
	// lock-free approach:
	// use atomic increment_exchange on the next available location in entsToTakeOver
	// then set the pointer there
	// if vector is at max capacity, must lock and resize - spin wait on other threads until there are available locations
}

void World::destroyEntity(Entity* e) {
#warning make Thread Safe!
	// lock-free approach - same as takeOwnership
	entsToDestroy.push_back(e);
#ifdef DEBUG
	// check if ent exists in vector
	assertDbg(std::find_if(entities.begin(), entities.end(), [e] (decltype(entities[0]) &x) {
		return x.get() == e;
	}) != entities.end() && "Entity is not managed by World!!!");
#endif
}

void World::destroyPending() {
	decltype(entsToDestroy) destroyNow;
	destroyNow.swap(entsToDestroy);
	for (auto &e : destroyNow) {
		auto it = std::find_if(entities.begin(), entities.end(), [&] (decltype(entities[0]) &it) {
			return it.get() == e;
		});
		if (it != entities.end()) {
			Entity::FunctionalityFlags flags = e->getFunctionalityFlags();
			if ((flags & Entity::FunctionalityFlags::UPDATABLE) != 0) {
				auto it = std::find(entsToUpdate.begin(), entsToUpdate.end(), e);
				assertDbg(it != entsToUpdate.end());
				entsToUpdate.erase(it);
			}
			if ((flags & Entity::FunctionalityFlags::DRAWABLE) != 0) {
				auto it = std::find(entsToDraw.begin(), entsToDraw.end(), e);
				assertDbg(it != entsToDraw.end());
				entsToDraw.erase(it);
			}
			entities.erase(it); // this will also delete
#warning "optimize this, it will be O(n^2) - must move the pointer from entities to entsToDestroy when destroy()"
		} else {
			ERROR("[WARNING] World skip DESTROY unmanaged obj: "<<e);
		}
	}
}

void World::takeOverPending() {
	decltype(entsToTakeOver) takeOverNow;
	takeOverNow.swap(entsToTakeOver);
	for (auto &e : takeOverNow) {
		// add to update and draw lists if appropriate
		Entity::FunctionalityFlags flags = e->getFunctionalityFlags();
		if ((flags & Entity::FunctionalityFlags::DRAWABLE) != 0) {
			entsToDraw.push_back(e.get());
		}
		if ((flags & Entity::FunctionalityFlags::UPDATABLE) != 0) {
			entsToUpdate.push_back(e.get());
		}
		e->managed_ = true;
		entities.push_back(std::move(e));
	}
}

void World::update(float dt) {
	// take over pending entities:
	takeOverPending();

	// delete pending entities:
	destroyPending();

	// do the actual update on entities:
#ifdef MT_UPDATE
	parallel_for(
#else
	std::for_each(
#endif
			entsToUpdate.begin(), entsToUpdate.end(),
#ifdef MT_UPDATE
			Infrastructure::getThreadPool(),
#endif
			[dt] (decltype(entsToUpdate[0]) &e) {
				e->update(dt);
			});
}

void World::draw(RenderContext const& ctx) {
	for (auto e : entsToDraw)
		e->draw(ctx);
}

bool World::testEntity(Entity &e, EntityType filterTypes, Entity::FunctionalityFlags filterFlags) {
	return (e.getEntityType() & filterTypes) != 0 &&
		((e.getFunctionalityFlags() & filterFlags) == filterFlags);
}


std::vector<Entity*> World::getEntities(EntityType filterTypes, Entity::FunctionalityFlags filterFlags) {
	std::vector<Entity*> vec;
	for (auto &e : entities) {
		if (!e->isZombie() && testEntity(*e, filterTypes, filterFlags))
			vec.push_back(e.get());
	}
	for (auto &e : entsToTakeOver) {
		if (!e->isZombie() && testEntity(*e, filterTypes, filterFlags))
			vec.push_back(e.get());
	}
	return vec;
}

std::vector<Entity*> World::getEntitiesInBox(EntityType filterTypes, Entity::FunctionalityFlags filterFlags, glm::vec2 pos, float radius, bool clipToCircle) {
	std::vector<b2Body*> bodies;
	getBodiesInArea(pos, radius, clipToCircle, bodies);
	std::vector<Entity*> out;
	for (b2Body* b : bodies) {
		PhysicsBody* pb = PhysicsBody::getForB2Body(b);
		if (pb == nullptr)
			continue;
		Entity* ent = pb->getAssociatedEntity();
		if (ent && !ent->isZombie() && testEntity(*ent, filterTypes, filterFlags))
			out.push_back(ent);
	}
	return out;
}

