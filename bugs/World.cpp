/*
 * World.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#include "World.h"
#include <algorithm>

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

void World::updatePrePhysics(float dt) {
	// rebuild rigid body cache:
	rigidBodiesCache.clear();
	for (auto obj : objects) {
		if (obj->getType() == TYPE_RIGID) {
			rigidBodiesCache.push_back(obj->getRigidBody());
		}
	}

	// rebuild spring cache:
	springsCache.clear();
	for (auto obj : objects) {
		if (obj->getType() == TYPE_SPRING) {
			springsCache.push_back(obj->getSpring());
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
