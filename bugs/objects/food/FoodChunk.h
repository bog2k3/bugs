/*
 * FoodChunk.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_FOOD_FOODCHUNK_H_
#define OBJECTS_FOOD_FOODCHUNK_H_

#include "../WorldObject.h"
#include "../../updatable.h"
#include "../../Event.h"

class FoodChunk: public WorldObject {
public:
	FoodChunk(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass);
	virtual ~FoodChunk() override;

	void draw(RenderContext& ctx) override;
	void update(float dt);

	Event<void(FoodChunk*)> onDestroy;

protected:
	float size_;
	float amountLeft_;
	float lifeTime_;
};

#endif /* OBJECTS_FOOD_FOODCHUNK_H_ */
