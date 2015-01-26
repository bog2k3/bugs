/*
 * Gamete.h
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#ifndef ENTITIES_GAMETE_H_
#define ENTITIES_GAMETE_H_

#include "Entity.h"
#include "../genetics/Genome.h"
#include "../PhysicsBody.h"

#define DEBUG_DRAW_GAMETE

class Gamete: public Entity {
public:
	Gamete(Chromosome &ch, glm::vec2 pos, glm::vec2 speed, float mass);
	virtual ~Gamete();
#ifdef DEBUG_DRAW_GAMETE
	FunctionalityFlags getFunctionalityFlags() override { return FF_DRAWABLE; }
#endif

	void update(float dt) override;
#ifdef DEBUG_DRAW_GAMETE
	void draw(RenderContext const& ctx) override;
#endif

protected:
	Chromosome chromosome_;
	PhysicsBody body_;

	void onCollision(PhysicsBody* pOther, float impulse);
};

#endif /* ENTITIES_GAMETE_H_ */
