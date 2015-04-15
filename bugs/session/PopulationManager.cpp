/*
 * PopulationManager.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: bogdan
 */

#include "PopulationManager.h"
#include "../entities/Bug.h"
#include "../utils/log.h"
#include "../utils/rand.h"
#include "../World.h"

static unsigned minPopulation = 10;				// minimum population number that triggers a refill
static unsigned refillPopulationTarget = 20;	// target population after refill

void PopulationManager::update(float dt) {
	if (Bug::getPopupationCount() != 0 && Bug::getPopupationCount() <= minPopulation) {
		LOGPREFIX("PopulationManager");
		LOGLN("Population reached the minimum point ("<<minPopulation<<"). Refilling up to "<<refillPopulationTarget<<"...");
		auto vec = World::getInstance()->getEntitiesOfType(Bug::entityType);
		unsigned spawnCount = refillPopulationTarget - Bug::getPopupationCount();
		for (unsigned i=0; i<spawnCount; i++) {
			int idx = randi(vec.size()-1);
			Bug* bug = static_cast<Bug*>(vec[idx]);
			glm::vec2 pos = glm::vec2(srandf()*(worldSize_.x-0.5f), srandf()*(worldSize_.y-0.5f));
			std::unique_ptr<Bug> newBug(new Bug(bug->getGenome(), bug->getMass(), pos, glm::vec2(0), bug->getGeneration()));
			World::getInstance()->takeOwnershipOf(std::move(newBug));
		}
	}
}

unsigned PopulationManager::getPopulationCount() {
	return Bug::getPopupationCount();
}

unsigned PopulationManager::getMaxGeneration() {
	return Bug::getMaxGeneration();
}
