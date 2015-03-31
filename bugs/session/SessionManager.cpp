/*
 * SessionManager.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#include "SessionManager.h"
#include "../World.h"
#include "../entities/Bug.h"
#include "../entities/food/FoodDispenser.h"
#include "../entities/Wall.h"
#include "../serialization/Serializer.h"
#include "../utils/rand.h"
#include <glm/vec2.hpp>

void SessionManager::startEmptySession() {
	World::getInstance()->free();
}

void SessionManager::startDefaultSession() {
	World::getInstance()->free();

	float worldRadius = 5.f;

	Wall* w1 = new Wall(glm::vec2(-worldRadius, -worldRadius), glm::vec2(+worldRadius, -worldRadius), 0.2f);
	World::getInstance()->takeOwnershipOf(w1);
	Wall* w2 = new Wall(glm::vec2(-worldRadius, +worldRadius), glm::vec2(+worldRadius, +worldRadius), 0.2f);
	World::getInstance()->takeOwnershipOf(w2);
	Wall* w3 = new Wall(glm::vec2(-worldRadius, -worldRadius), glm::vec2(-worldRadius, +worldRadius), 0.2f);
	World::getInstance()->takeOwnershipOf(w3);
	Wall* w4 = new Wall(glm::vec2(+worldRadius, -worldRadius), glm::vec2(+worldRadius, +worldRadius), 0.2f);
	World::getInstance()->takeOwnershipOf(w4);

	for (int i=0; i<15; i++) {
		FoodDispenser* foodDisp = new FoodDispenser(glm::vec2(srandf()*(worldRadius-0.5f), srandf()*(worldRadius-0.5f)), 0);
		World::getInstance()->takeOwnershipOf(foodDisp);
	}

	for (int i=0; i<20; i++) {
#warning "crash in fixGenesSynchro on basicMutantBug"
		Bug* bug = Bug::newBasicMutantBug(glm::vec2(srandf()*(worldRadius-0.5f), srandf()*(worldRadius-0.5f)));
		//Bug* bug = Bug::newBasicBug(glm::vec2(srandf()*(worldRadius-0.5f), srandf()*(worldRadius-0.5f)));
		//if (i==8)
			World::getInstance()->takeOwnershipOf(bug);
	}
}

void SessionManager::loadSessionFromFile(std::string const &path) {
	World::getInstance()->free();
	mergeSessionFromFile(path);
}

void SessionManager::mergeSessionFromFile(std::string const &path) {
	Serializer serializer;
	serializer.deserializeFromFile(path);
}
