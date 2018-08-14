/*
 * PopulationManager.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: bogdan
 */

#include "PopulationManager.h"
#include "../entities/Bug/Bug.h"

#include <boglfw/World.h>
#include <boglfw/perf/marker.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/rand.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

#define DEBUG_ONE_BUG_ONLY

#ifdef DEBUG
	#ifdef DEBUG_ONE_BUG_ONLY
		static unsigned minPopulation = 0;				// minimum population number that triggers a refill
		static unsigned refillPopulationTarget = 1;		// target population after refill
	#else
		static unsigned minPopulation = 15;				// minimum population number that triggers a refill
		static unsigned refillPopulationTarget = 40;	// target population after refill
	#endif
#else
	static unsigned minPopulation = 15;				// minimum population number that triggers a refill
	static unsigned refillPopulationTarget = 60;	// target population after refill
#endif

void PopulationManager::setPopulationTarget(unsigned thresh, unsigned refill) {
	refillPopulationTarget = refill;
	minPopulation = thresh;
}

unsigned PopulationManager::getPopulationTarget() {
	return refillPopulationTarget;
}

void PopulationManager::update(float dt) {
	PERF_MARKER_FUNC;
	unsigned bugPopulation = Bug::getPopulationCount() + Bug::getZygotesCount();
	if (bugPopulation != 0 && bugPopulation <= minPopulation) {
		LOGPREFIX("PopulationManager");
		LOGLN("Population reached the minimum point ("<<minPopulation<<"). Refilling up to "<<refillPopulationTarget<<"...");
//		auto vec = World::getInstance().getEntitiesOfType(Bug::entityType);
//		vec.erase(std::remove_if(vec.begin(), vec.end(), [] (Entity* e) {
//			Bug* bug = static_cast<Bug*>(e);
//			return !bug->isAlive();
//		}), vec.end());
		unsigned spawnCount = refillPopulationTarget - bugPopulation;
		for (unsigned i=0; i<spawnCount; i++) {
//			int idx = randi(vec.size()-1);
//			Bug* bug = static_cast<Bug*>(vec[idx]);
			glm::vec2 pos = glm::vec2(srandf()*worldSize_.x*0.5f, srandf()*worldSize_.y*0.5f);
//			std::unique_ptr<Bug> newBug(new Bug(bug->getGenome(), bug->getMass(), pos, glm::vec2(0), bug->getGeneration()));
			std::unique_ptr<Bug> newBug(Bug::newBasicMutantBug(pos));
			World::getInstance().takeOwnershipOf(std::move(newBug));
		}
	}
}

unsigned PopulationManager::getPopulationCount() {
	return Bug::getPopulationCount();
}

unsigned PopulationManager::getMaxGeneration() {
	return Bug::getMaxGeneration();
}
