/*
 * World.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#include "World.h"
#include <glm/glm.hpp>
#include <algorithm>
#include "math/math2D.h"

World::World() {
	// TODO Auto-generated constructor stub

}

World::~World() {
	// TODO Auto-generated destructor stub
}

void World::retrieveObjectsInCircle(Circle const &circle, std::vector<RigidBody*> &outVector) {

}

void World::retrieveObjects(std::vector<RigidBody*> &outVector) {
	outVector.reserve(outVector.size() + rigidBodiesCache.size());
	outVector.insert(outVector.end(), rigidBodiesCache.begin(), rigidBodiesCache.end());
}

void World::retrieveObjects(std::vector<Spring*> &outVector) {
	outVector.reserve(outVector.size() + springsCache.size());
	outVector.insert(outVector.end(), springsCache.begin(), springsCache.end());
}

WorldObject* World::getObjectAtPos(glm::vec2 pos) {
	return *objects.begin();
}
void World::getObjectsInBox(AlignedBox box, std::vector<WorldObject*> &outVec) {

}

void World::updatePrePhysics(float dt) {
	// rebuild caches:
	rigidBodiesCache.clear();
	springsCache.clear();

	for (auto obj : objects) {
		switch (obj->getType()) {
		case TYPE_RIGID:
			rigidBodiesCache.push_back(obj->getRigidBody());
			break;
		case TYPE_SPRING:
			springsCache.push_back(obj->getSpring());
			break;
		}
	}
}

void World::updatePostPhysics(float dt) {
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
