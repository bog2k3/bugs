/*
 * FoodChunk.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_FOOD_FOODCHUNK_H_
#define OBJECTS_FOOD_FOODCHUNK_H_

#include "../Entity.h"
#include "../enttypes.h"
#include "../../utils/Event.h"
#include "../../PhysicsBody.h"

#define DEBUG_DRAW_FOOD_CHUNK

class FoodChunk: public Entity {
public:
	FoodChunk(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass);
	virtual ~FoodChunk() override;
	FunctionalityFlags getFunctionalityFlags() override {
		return FF_UPDATABLE | FF_DRAWABLE;
	}
	static constexpr EntityType entityType = ENTITY_FOOD_CHUNK;
	virtual EntityType getEntityType() override { return entityType; }

	void update(float dt) override;

#ifdef DEBUG_DRAW_FOOD_CHUNK
	void draw(RenderContext const& rc) override;
#endif

	float getInitialMass() { return initialMass_; }
	float getMassLeft() { return amountLeft_; }
	void consume(float massAmount);

	Event<void(FoodChunk*)> onDestroy;

protected:
	PhysicsBody physBody_;
	float size_;
	float initialMass_;
	float amountLeft_;
};

#endif /* OBJECTS_FOOD_FOODCHUNK_H_ */
