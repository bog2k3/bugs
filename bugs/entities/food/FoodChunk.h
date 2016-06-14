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
#include "../../physics/PhysicsBody.h"
#include "../../utils/Event.h"
#include "../../utils/bitFlags.h"

#define DEBUG_DRAW_FOOD_CHUNK

class FoodChunk: public Entity {
public:
	FoodChunk(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass);
	virtual ~FoodChunk() override;
	FunctionalityFlags getFunctionalityFlags() override { return
			FunctionalityFlags::UPDATABLE |
			FunctionalityFlags::DRAWABLE;
	}
	static constexpr EntityType entityType = EntityType::FOOD_CHUNK;
	virtual EntityType getEntityType() override { return entityType; }
	glm::vec3 getWorldTransform() override;
	aabb getAABB() const override;

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

private:
	static Entity* getEntityFromFoodChunkPhysBody(PhysicsBody const& body);
};

#endif /* OBJECTS_FOOD_FOODCHUNK_H_ */
