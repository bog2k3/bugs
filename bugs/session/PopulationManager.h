/*
 * PopulationManager.h
 *
 *  Created on: Apr 15, 2015
 *      Author: bogdan
 */

#ifndef SESSION_POPULATIONMANAGER_H_
#define SESSION_POPULATIONMANAGER_H_

#include <glm/vec2.hpp>

class PopulationManager {
public:
	void udate(float dt);
	void setWorldSize(glm::vec2 size) { worldSize_ = size; }

private:
	glm::vec2 worldSize_{0};
};

#endif /* SESSION_POPULATIONMANAGER_H_ */
