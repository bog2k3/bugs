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

WorldObject* World::getObjectAtPos(glm::vec2 pos) {
	return *objects.begin();
}
void World::getObjectsInBox(AlignedBox box, std::vector<WorldObject*> &outVec) {

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
