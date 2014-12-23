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
	// TODO Auto-generated destructor stub
}

bool World::ReportFixture(b2Fixture* fixture) {
	b2QueryResult.push_back(fixture);
	return true;
}

WorldObject* World::getObjectAtPos(glm::vec2 pos) {
	assert(b2QueryResult.empty());
	b2AABB aabb;
	aabb.lowerBound = g2b(pos) - b2Vec2(0.005f, 0.005f);
	aabb.upperBound = g2b(pos) + b2Vec2(0.005f, 0.005f);
	physWld->QueryAABB(this, aabb);
	if (b2QueryResult.empty())
		return nullptr;
	WorldObject* ret = nullptr;
	for (b2Fixture* f : b2QueryResult) {
		if (f->TestPoint(g2b(pos))) {
			ret = (WorldObject*)f->GetBody()->GetUserData();
			break;
		}
	}
	b2QueryResult.clear();	// reset
	return ret;
}
void World::getObjectsInBox(AlignedBox box, std::vector<WorldObject*> &outVec) {
	b2AABB aabb;
	aabb.lowerBound = g2b(box.bottomLeft);
	aabb.upperBound = g2b(box.topRight);
	physWld->QueryAABB(this, aabb);
	while (!b2QueryResult.empty()) {
		outVec.push_back((WorldObject*)b2QueryResult.back()->GetBody()->GetUserData());
		b2QueryResult.pop_back();
	}
}

template<> void draw(World*& wld, RenderContext& ctx) {
	for (auto obj : wld->objects) {
		obj->draw(ctx);
	}
}

void World::addObject(WorldObject* obj) {
	objects.push_back(obj);
}

void World::removeObject(WorldObject* obj) {
	objects.erase(
			std::remove_if(objects.begin(), objects.end(),
					[obj] (WorldObject* const & x) { return x == obj; }),
			objects.end()
		);
}
