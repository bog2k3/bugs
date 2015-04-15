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

void PopulationManager::udate(float dt) {
	if (Bug::getPopupationCount() <= minPopulation) {
		LOGPREFIX("PopulationManager");
		LOGLN("Population reached the minimum point ("<<minPopulation<<"). Refilling up to "<<refillPopulationTarget<<"...");
		auto vec = World::getInstance()->getEntitiesOfType(Bug::entityType);
		unsigned spawnCount = refillPopulationTarget - Bug::getPopupationCount();
		for (int i=0; i<spawnCount; i++) {
			int idx = randi(vec.size()-1);
			glm::vec2 pos = glm::vec2(srandf()*(worldSize_.x-0.5f), srandf()*(worldSize_.y-0.5f));
			std::unique_ptr<Bug> newBug(new Bug(vec[idx]->getGenome(), vec[idx]->getMass(), pos, glm::vec2(0), vec[idx]->getGeneration()));
		}
	}
}
