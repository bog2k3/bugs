/*
 * SessionManager.h
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#ifndef SESSION_SESSIONMANAGER_H_
#define SESSION_SESSIONMANAGER_H_

#include "PopulationManager.h"
#include <string>

class SessionManager {
public:
	SessionManager();
	virtual ~SessionManager() = default;

	void startEmptySession();
	void startDefaultSession();
	bool loadSessionFromFile(std::string const &path);
	bool mergeSessionFromFile(std::string const &path);
	bool saveSessionToFile(std::string const &path);

	PopulationManager& getPopulationManager() { return populationMgr; }

private:
	PopulationManager populationMgr;
};

#endif /* SESSION_SESSIONMANAGER_H_ */
