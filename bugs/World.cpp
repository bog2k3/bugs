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
#include "Infrastructure.h"
#include "renderOpenGL/Shape2D.h"

#include "utils/bitFlags.h"
#include "utils/parallel.h"
#include "utils/assert.h"
#include "utils/log.h"

#include "../perf/marker.h"

#include <glm/glm.hpp>
#include <Box2D/Box2D.h>
#include <algorithm>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

#define MT_UPDATE	// enables parallel update on entities, using the thread pool

static World *instance = nullptr;

World::World()
	: physWld(nullptr)
	, groundBody(nullptr)
	, entsToDestroy(1024)
	, entsToTakeOver(1024)
	, deferredActions_(4096)
{
	assert(instance == nullptr && "attempting to initialize multiple instances of World!!!");
	instance = this;
#ifdef DEBUG
	ownerThreadId_ = std::this_thread::get_id();
#endif
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

void World::setBounds(float left, float right, float top, float bottom) {
	extentXn_ = left;
	extentXp_ = right;
	extentYp_ = top;
	extentYn_ = bottom;
	// reconfigure cache:
	spatialCache_ = SpatialCache(left, right, top, bottom);
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

void World::getFixtures(std::vector<b2Fixture*> &out, const b2AABB& aabb) {
	PERF_MARKER_FUNC;
	class cbWrap : public b2QueryCallback {
	public:
		cbWrap(std::vector<b2Fixture*> &fixtures) : fixtures_(fixtures) {}
		/// b2QueryCallback::
		/// Called for each fixture found in the query AABB.
		/// @return false to terminate the query.
		bool ReportFixture(b2Fixture* fixture) override {
			fixtures_.push_back(fixture);
			return true;
		}

		std::vector<b2Fixture*> &fixtures_;
	} wrap(out);
	physWld->QueryAABB(&wrap, aabb);
}

b2Body* World::getBodyAtPos(glm::vec2 const& pos) {
	PERF_MARKER_FUNC;
	b2AABB aabb;
	aabb.lowerBound = g2b(pos) - b2Vec2(0.005f, 0.005f);
	aabb.upperBound = g2b(pos) + b2Vec2(0.005f, 0.005f);
	static thread_local std::vector<b2Fixture*> b2QueryResult;
	b2QueryResult.clear();
	getFixtures(b2QueryResult, aabb);
	if (b2QueryResult.empty())
		return nullptr;
	PERF_MARKER("precisionTest");
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
	PERF_MARKER_FUNC;
	b2AABB aabb;
	aabb.lowerBound = g2b(pos) - b2Vec2(radius, radius);
	aabb.upperBound = g2b(pos) + b2Vec2(radius, radius);
	static thread_local std::vector<b2Fixture*> b2QueryResult;
	b2QueryResult.clear();
	getFixtures(b2QueryResult, aabb);
	for (b2Fixture* f : b2QueryResult) {
		if (clipToCircle) {
			PERF_MARKER("clipToCircle");
			if (glm::length(b2g(f->GetAABB(0).GetCenter()) - pos) > radius)
				continue;
		}
		outBodies.push_back(f->GetBody());
	}
}

void World::takeOwnershipOf(std::unique_ptr<Entity> &&e) {
	assertDbg(e != nullptr);
	e->managed_ = true;
	entsToTakeOver.push_back(std::move(e));
}

void World::destroyEntity(Entity* e) {
	PERF_MARKER_FUNC;
	entsToDestroy.push_back(e);
#ifdef DEBUG
	// check if ent exists in vector
	assertDbg(std::find_if(entities.begin(), entities.end(), [e] (decltype(entities[0]) &x) {
		return x.get() == e;
	}) != entities.end() && "Entity is not managed by World!!!");
#endif
}

void World::destroyPending() {
	PERF_MARKER_FUNC;
	static decltype(entsToDestroy) destroyNow(entsToDestroy.getLockFreeCapacity());
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
	destroyNow.clear();
}

void World::takeOverPending() {
	PERF_MARKER_FUNC;
	static decltype(entsToTakeOver) takeOverNow(entsToTakeOver.getLockFreeCapacity());
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
		entities.push_back(std::move(e));
	}
	takeOverNow.clear();
}

void World::update(float dt) {
	PERF_MARKER_FUNC;
	++frameNumber_;

	// take over pending entities:
	takeOverPending();

	// delete pending entities:
	destroyPending();

	// do the actual update on entities:
	do {
	PERF_MARKER("entities-update");
#ifdef MT_UPDATE
	parallel_for(
#else
	std::for_each(
#endif
			entsToUpdate.begin(), entsToUpdate.end(),
#ifdef MT_UPDATE
			Infrastructure::getThreadPool(),
#endif
			[dt] (auto &e) {
				e->update(dt);
			});
	} while (0);

	// execute deferred actions synchronously:
	{
		PERF_MARKER("deferred-actions");
		executingDeferredActions_.store(true, std::memory_order_release);
		for (auto &a : deferredActions_)
			a();
		deferredActions_.clear();
		executingDeferredActions_.store(false, std::memory_order_release);
	}
}

void World::queueDeferredAction(std::function<void()> &&fun) {
	if (executingDeferredActions_)
		fun();
	else
		deferredActions_.push_back(std::move(fun));
}

void World::draw(RenderContext const& ctx) {
	PERF_MARKER_FUNC;
	// draw extent lines:
	glm::vec3 lineColor(0.2f, 0, 0.8f);
	ctx.shape->drawLine(glm::vec2(extentXn_, extentYp_*1.5f), glm::vec2(extentXn_, extentYn_*1.5f), 0, lineColor);
	ctx.shape->drawLine(glm::vec2(extentXp_, extentYp_*1.5f), glm::vec2(extentXp_, extentYn_*1.5f), 0, lineColor);
	ctx.shape->drawLine(glm::vec2(extentXn_*1.5f, extentYp_), glm::vec2(extentXp_*1.5f, extentYp_), 0, lineColor);
	ctx.shape->drawLine(glm::vec2(extentXn_*1.5f, extentYn_), glm::vec2(extentXp_*1.5f, extentYn_), 0, lineColor);
	// draw entities
	for (auto e : entsToDraw)
		e->draw(ctx);
}

bool World::testEntity(Entity &e, EntityType filterTypes, Entity::FunctionalityFlags filterFlags) {
	return (e.getEntityType() & filterTypes) != 0 &&
		((e.getFunctionalityFlags() & filterFlags) == filterFlags);
}


void World::getEntities(std::vector<Entity*> &out, EntityType filterTypes, Entity::FunctionalityFlags filterFlags) {
	PERF_MARKER_FUNC;
	for (auto &e : entities) {
		if (!e->isZombie() && testEntity(*e, filterTypes, filterFlags))
			out.push_back(e.get());
	}
	for (auto &e : entsToTakeOver) {
		if (!e->isZombie() && testEntity(*e, filterTypes, filterFlags))
			out.push_back(e.get());
	}
}

void World::getEntitiesInBox(std::vector<Entity*> &out, EntityType filterTypes, Entity::FunctionalityFlags filterFlags,
		glm::vec2 const& pos, float radius, bool clipToCircle)
{
	PERF_MARKER_FUNC;
	spatialCache_.getCachedEntities(out, pos, radius, clipToCircle, frameNumber_,
		[this, filterTypes, filterFlags] (glm::vec2 const& pos, float radius, std::vector<Entity*> &out)
	{
		static thread_local std::vector<b2Body*> bodies;
		bodies.clear();
		getBodiesInArea(pos, radius, false, bodies);
		for (b2Body* b : bodies) {
			PhysicsBody* pb = PhysicsBody::getForB2Body(b);
			if (pb == nullptr)
				continue;
			Entity* ent = pb->getAssociatedEntity();
			if (ent && !ent->isZombie())
				out.push_back(ent);
		}
	}, [this, filterTypes, filterFlags] (Entity *e) {
		return testEntity(*e, filterTypes, filterFlags);
	});
}

