/*
 * Spring.h
 *
 *  Created on: Nov 10, 2014
 *      Author: bogdan
 */

#ifndef PHYSICS_SPRING_H_
#define PHYSICS_SPRING_H_

#include "AttachPoint.h"

class Spring {
public:
	const AttachPoint a1;
	const AttachPoint a2;

	const float k;
	const float initialLength;

	Spring(AttachPoint a1, AttachPoint a2, float k, float initialLength);
	virtual ~Spring();

	// returns the force vector, oriented from attachment point #1 towards #2
	glm::vec2 getForce();
};

#endif /* PHYSICS_SPRING_H_ */
