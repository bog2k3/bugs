/*
 * World.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#include "World.h"
#include "math/math2D.h"
#include "math/box2glm.h"
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
	// TODO delete all entities
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
	entities.push_back(e);
	// add to update and draw lists if appropriate
}
