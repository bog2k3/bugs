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

	void update(float dt);

	// returns the force vector, oriented from attachment point #1 towards #2
	glm::vec2 getForce() { return force; }
	float getDelta();
private:
	glm::vec2 force;	// averaged force over the last frame
	glm::vec2 f0;		// instant force at previous sampling point
	float l0;			// elongation at last frame
	float v0;			// dx' = speed of change in elongation
	float a0;			// dx'' = speed of change in v0
};

#endif /* PHYSICS_SPRING_H_ */
