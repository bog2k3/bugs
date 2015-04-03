/*
 * SessionManager.h
 *
 *  Created on: Mar 31, 2015
 *      Author: bog
 */

#ifndef SESSION_SESSIONMANAGER_H_
#define SESSION_SESSIONMANAGER_H_

#include <string>

class SessionManager {
public:
	SessionManager();
	virtual ~SessionManager() = default;

	void startEmptySession();
	void startDefaultSession();
	void loadSessionFromFile(std::string const &path);
	void mergeSessionFromFile(std::string const &path);
	void saveSessionToFile(std::string const &path);
};

#endif /* SESSION_SESSIONMANAGER_H_ */
