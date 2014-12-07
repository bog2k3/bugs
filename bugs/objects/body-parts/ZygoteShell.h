/*
 * ZygoteShell.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_ZYGOTESHELL_H_
#define OBJECTS_BODY_PARTS_ZYGOTESHELL_H_

#include "BodyPart.h"

class ZygoteShell: public BodyPart {
public:
	ZygoteShell(World* world, glm::vec2 position, float angle, bool dynamic, glm::vec2 velocity, float angularVelocity);
	virtual ~ZygoteShell();
};

#endif /* OBJECTS_BODY_PARTS_ZYGOTESHELL_H_ */
