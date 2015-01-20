/*
 * FoodDispenser.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_FOOD_FOODDISPENSER_H_
#define OBJECTS_FOOD_FOODDISPENSER_H_

#include "../WorldObject.h"
#include "../../updatable.h"

class FoodDispenser: public WorldObject {
public:
	FoodDispenser(glm::vec2 position, float direction);
	virtual ~FoodDispenser();

	void draw(RenderContext& ctx) override;

	void update(float dt);

protected:
	float direction_;
	float period_;
	float timer_;
	glm::vec2 spawnPosition_;
	glm::vec2 spawnDirection_;
	float spawnVelocity_;
	float spawnMass_;
};

template<> void update(FoodDispenser*& disp, float dt);

#endif /* OBJECTS_FOOD_FOODDISPENSER_H_ */
