/*
 * SessionManager.cpp
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#include "SessionManager.h"
#include "../World.h"
#include "../entities/Entity.h"
#include "../entities/Bug.h"
#include "../entities/food/FoodDispenser.h"
#include "../entities/food/FoodChunk.h"
#include "../entities/Gamete.h"
#include "../entities/Wall.h"
#include "../serialization/Serializer.h"
#include "../utils/rand.h"
#include "../utils/log.h"
#include <glm/vec2.hpp>

SessionManager::SessionManager() {
	Serializer::setDeserializationObjectMapping(SerializationObjectTypes::BUG, &Bug::deserialize);
	Serializer::setDeserializationObjectMapping(SerializationObjectTypes::GAMETE, &Gamete::deserialize);
	Serializer::setDeserializationObjectMapping(SerializationObjectTypes::FOOD_DISPENSER, &FoodDispenser::deserialize);
	// Serializer::setDeserializationObjectMapping(SerializationObjectTypes::GENOME, &Bug::deserializeGenome!??!?!);

}

void SessionManager::startEmptySession() {
	LOGGER("SessionManager");
	LOGLN("Starting empty session... removing all existing entities...");
	World::getInstance()->free();
	LOGLN("Finished. Session is now clean.");
}

void SessionManager::startDefaultSession() {
	LOGGER("SessionManager");
	LOGLN("Creating default session...");
	//LOGLN("Removing all entities...");
	World::getInstance()->free();
	//LOGLN("World is now clean.");

	LOGLN("Building entities for default session...");
	float worldRadius = 5.f;

	std::unique_ptr<Wall> w1(new Wall(glm::vec2(-worldRadius, -worldRadius), glm::vec2(+worldRadius, -worldRadius), 0.2f));
	World::getInstance()->takeOwnershipOf(std::move(w1));
	std::unique_ptr<Wall> w2(new Wall(glm::vec2(-worldRadius, +worldRadius), glm::vec2(+worldRadius, +worldRadius), 0.2f));
	World::getInstance()->takeOwnershipOf(std::move(w2));
	std::unique_ptr<Wall> w3(new Wall(glm::vec2(-worldRadius, -worldRadius), glm::vec2(-worldRadius, +worldRadius), 0.2f));
	World::getInstance()->takeOwnershipOf(std::move(w3));
	std::unique_ptr<Wall> w4(new Wall(glm::vec2(+worldRadius, -worldRadius), glm::vec2(+worldRadius, +worldRadius), 0.2f));
	World::getInstance()->takeOwnershipOf(std::move(w4));

	for (int i=0; i<15; i++) {
		std::unique_ptr<FoodDispenser> foodDisp(new FoodDispenser(glm::vec2(srandf()*(worldRadius-0.5f), srandf()*(worldRadius-0.5f)), 0));
		World::getInstance()->takeOwnershipOf(std::move(foodDisp));
	}

	for (int i=0; i<20; i++) {
#warning "crash in fixGenesSynchro on basicMutantBug"
		//std::unique_ptr<Bug> bug(Bug::newBasicMutantBug(glm::vec2(srandf()*(worldRadius-0.5f), srandf()*(worldRadius-0.5f))));
		std::unique_ptr<Bug> bug(Bug::newBasicBug(glm::vec2(srandf()*(worldRadius-0.5f), srandf()*(worldRadius-0.5f))));
		//if (i==8)
			World::getInstance()->takeOwnershipOf(std::move(bug));
	}
	LOGLN("Finished building default session.");
}

void SessionManager::loadSessionFromFile(std::string const &path) {
	LOGGER("SessionManager");
	LOGLN("Loading session from file \"" << path << "\"...");
	// LOGLN("Removing all entities...");
	World::getInstance()->free();
	// LOGLN("World is now clean.");
	mergeSessionFromFile(path);
}

void SessionManager::mergeSessionFromFile(std::string const &path) {
	LOGGER("SessionManager");
	LOGLN("Merging session from file \"" << path << "\"...");
	Serializer serializer;
	if (!serializer.deserializeFromFile(path)) {
		LOGLN("WARNING: There was an error during deserialization of the session file.");
		return;
	}
	LOGLN("Finished merging.");
}

void SessionManager::saveSessionToFile(std::string const& path) {
	LOGGER("SessionManager");
	LOGLN("Saving session to file \"" << path << "\"...");
	Serializer serializer;
	auto vecSer = World::getInstance()->getEntities(Entity::FF_SERIALIZABLE);
	for (auto e : vecSer)
		serializer.queueObject(e);
	serializer.serializeToFile(path);
	LOGLN("Finished saving.");
}
