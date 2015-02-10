/*
 * FoodDispenser.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_FOOD_FOODDISPENSER_H_
#define OBJECTS_FOOD_FOODDISPENSER_H_

#include "../Entity.h"
#include "../../PhysicsBody.h"

class FoodDispenser: public Entity {
public:
	FoodDispenser(glm::vec2 position, float direction);
	virtual ~FoodDispenser();
	FunctionalityFlags getFunctionalityFlags() override { return FF_UPDATABLE; }

	void draw(RenderContext const& ctx) override;
	void update(float dt) override;

protected:
	PhysicsBody physBody_;
	float radius_;
	glm::vec2 position_;
	float direction_;
	float period_;
	float timer_;
	float spawnVelocity_;
	float spawnMass_;
};

#endif /* OBJECTS_FOOD_FOODDISPENSER_H_ */