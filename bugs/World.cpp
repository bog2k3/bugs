/*
 * World.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#include "World.h"

World::World() {
	// TODO Auto-generated constructor stub

}

World::~World() {
	// TODO Auto-generated destructor stub
}

void World::retrieveObjectsInCircle(Circle const &circle, std::vector<RigidBody*> &outVector) {

}

void World::retrieveObjects(std::vector<RigidBody*> &outVector) {

}

void World::retrieveObjects(std::vector<Spring*> &outVector) {

}

void World::update(float dt) {
}

void World::draw() {
	for (auto obj : objects) {
		obj->draw(&renderContext);
	}
}

void World::addObject(IWorldObject* obj) {
	objects.push_back(obj);
}
