/*
 * Torso.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_TORSO_H_
#define OBJECTS_BODY_PARTS_TORSO_H_

#include "BodyPart.h"

class Torso : public BodyPart {
public:
	Torso(BodyPart* parent, float size, PhysicsProperties props);
	virtual ~Torso();
};

#endif /* OBJECTS_BODY_PARTS_TORSO_H_ */
