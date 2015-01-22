/*
 * FoodChunk.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_FOOD_FOODCHUNK_H_
#define OBJECTS_FOOD_FOODCHUNK_H_

#include "../Entity.h"
#include "../../Event.h"
#include "../../PhysicsBody.h"

class FoodChunk: public Entity {
public:
	FoodChunk(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass);
	virtual ~FoodChunk() override;
	FunctionalityFlags getFunctionalityFlags() override {
		return FF_UPDATABLE | FF_DRAWABLE;
	}

	void update(float dt) override;
	void draw(RenderContext const& rc) override;

	Event<void(FoodChunk*)> onDestroy;

protected:
	PhysicsBody physBody_;
	float size_;
	float amountLeft_;
	float lifeTime_;
};

#endif /* OBJECTS_FOOD_FOODCHUNK_H_ */
