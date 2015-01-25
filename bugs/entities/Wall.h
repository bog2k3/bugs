/*
 * Wall.h
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#ifndef ENTITIES_WALL_H_
#define ENTITIES_WALL_H_

#include "Entity.h"
#include "../PhysicsBody.h"
#include <glm/vec2.hpp>

class Wall : public Entity {
public:
	Wall(glm::vec2 from, glm::vec2 to, float width);
	virtual ~Wall();

protected:
	PhysicsBody body_;
};

#endif /* ENTITIES_WALL_H_ */
