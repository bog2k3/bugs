/*
 * FoodDispenser.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_FOOD_FOODDISPENSER_H_
#define OBJECTS_FOOD_FOODDISPENSER_H_

#include "../WorldObject.h"
#include "../../Event.h"
#include "../../UpdateList.h"

class FoodDispenser: public WorldObject {
public:
	virtual ~FoodDispenser();

	Event<void(FoodDispenser*)> onDestroy;

	void draw(RenderContext& ctx) override;
	void update(float dt);

protected:
	FoodDispenser(glm::vec2 position, float direction);
	friend class World;
	friend shared_ptr<FoodDispenser> std::make_shared(...);

	float radius_;
	glm::vec2 position_;
	float direction_;
	float period_;
	float timer_;
	float spawnVelocity_;
	float spawnMass_;

	UpdateList updateList_;
};

#endif /* OBJECTS_FOOD_FOODDISPENSER_H_ */
