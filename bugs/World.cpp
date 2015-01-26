/*
 * World.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#include "World.h"
#include "entities/Entity.h"
#include "math/math2D.h"
#include "math/box2glm.h"
#include "utils/log.h"
#include <glm/glm.hpp>
#include <Box2D/Box2D.h>
#include <algorithm>

World::World() : physWld(nullptr), groundBody(nullptr) {
}

void World::setPhysics(b2World* phys) {
	physWld = phys;
	b2BodyDef gdef;
	gdef.type = b2_staticBody;
	groundBody = physWld->CreateBody(&gdef);
}

World* World::getInstance() {
	static World instance;
	return &instance;
}

World::~World() {
	free();
}

void World::free() {
	for (Entity* e : entities) {
		e->markedForDeletion_ = true;
		delete e;
	}
	for (Entity* e : entsToTakeOver) {
		e->markedForDeletion_ = true;
		delete e;
	}
	entities.clear();
	entsToTakeOver.clear();
	entsToDestroy.clear();
	entsToDraw.clear();
	entsToUpdate.clear();
}

bool World::ReportFixture(b2Fixture* fixture) {
	b2QueryResult.push_back(fixture);
	return true;
}

b2Body* World::getBodyAtPos(glm::vec2 pos) {
	assert(b2QueryResult.empty());
	b2AABB aabb;
	aabb.lowerBound = g2b(pos) - b2Vec2(0.005f, 0.005f);
	aabb.upperBound = g2b(pos) + b2Vec2(0.005f, 0.005f);
	physWld->QueryAABB(this, aabb);
	if (b2QueryResult.empty())
		return nullptr;
	b2Body* ret = nullptr;
	for (b2Fixture* f : b2QueryResult) {
		if (f->TestPoint(g2b(pos))) {
			ret = f->GetBody();
			break;
		}
	}
	b2QueryResult.clear();	// reset
	return ret;
}

void World::takeOwnershipOf(Entity* e) {
	entsToTakeOver.push_back(e);
	// LOGLN("QUEUE IN ENT: "<<e);
}

void World::destroyEntity(Entity* e) {
	entsToDestroy.push_back(e);
	// LOGLN("QUEUE OUT ENT: "<< e);
}

void World::destroyPending() {
	for (auto e : entsToDestroy) {
		Entity::FunctionalityFlags flags = e->getFunctionalityFlags();
		if (flags & Entity::FF_UPDATABLE) {
			auto it = std::find(entsToUpdate.begin(), entsToUpdate.end(), e);
			if (it != entsToUpdate.end()) {
				entsToUpdate.erase(it);
				//LOGLN("OUT UPDATABLE ENT: "<<e);
			} else
				ERROR("updatable entity not found in vector!!!");
		}
		if (flags & Entity::FF_DRAWABLE) {
			auto it = std::find(entsToDraw.begin(), entsToDraw.end(), e);
			if (it != entsToDraw.end()) {
				entsToDraw.erase(it);
				//LOGLN("OUT DRAWABLE ENT: "<<e);
			} else
				ERROR("drawable entity not found in vector!!!");
		}
		auto pos = std::find(entities.begin(), entities.end(), e);
		if (pos != entities.end()) {
			entities.erase(pos);
			//LOGLN("OUT ENT: "<<e);
		} else
			ERROR("entity not found in vector!!!");
		//LOGLN("DELETE ENT: " << e);
		delete e;
	}
	entsToDestroy.clear();
}

void World::takeOverPending() {
	for (auto e : entsToTakeOver) {
		//LOGLN("IN ENT: "<<e);
		entities.push_back(e);
		// add to update and draw lists if appropriate
		Entity::FunctionalityFlags flags = e->getFunctionalityFlags();
		if (flags & Entity::FF_DRAWABLE) {
			//LOGLN("IN DRAWABLE ENT: "<<e);
			entsToDraw.push_back(e);
		}
		if (flags & Entity::FF_UPDATABLE) {
			//LOGLN("IN UPDATABLE ENT: "<<e);
			entsToUpdate.push_back(e);
		}
	}
	entsToTakeOver.clear();
}

void World::update(float dt) {
	// delete pending entities:
	destroyPending();

	// take over pending entities:
	takeOverPending();

	// do the actual update on entities:
	for (auto e : entsToUpdate)
		e->update(dt);
}

void World::draw(RenderContext const& ctx) {
	for (auto e : entsToDraw)
		e->draw(ctx);
}
