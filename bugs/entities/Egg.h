/*
 * Egg.h
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#ifndef ENTITIES_EGG_H_
#define ENTITIES_EGG_H_

#include "Entity.h"
#include "../genetics/Genome.h"
#include "../PhysicsBody.h"

class Egg: public Entity {
public:
	Egg(Chromosome &ch, glm::vec2 pos, glm::vec2 speed, float mass);
	virtual ~Egg();

protected:
	Chromosome chromosome_;
	PhysicsBody body_;

	void onCollision(PhysicsBody* pOther, float impulse);
};

#endif /* ENTITIES_EGG_H_ */
