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

World::World(b2World* physWld) : physWld(physWld) {
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
	aabb.lowerBound = aabb.upperBound = g2b(pos);
	physWld->QueryAABB(this, aabb);
	b2Fixture* ptr = b2QueryResult.front();
	b2QueryResult.clear();	// reset
	return (WorldObject*)ptr->GetBody()->GetUserData();
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

void World::draw() {
	for (auto obj : objects) {
		obj->draw(&renderContext);
	}
}

void World::addObject(WorldObject* obj) {
	objects.push_back(obj);
}

void World::removeObject(WorldObject* obj) {
	objects.remove_if([obj] (WorldObject* const & x) { return x == obj; });
}
