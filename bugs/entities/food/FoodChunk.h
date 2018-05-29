/*
 * FoodChunk.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_FOOD_FOODCHUNK_H_
#define OBJECTS_FOOD_FOODCHUNK_H_

#include "../enttypes.h"

#include <boglfw/entities/Entity.h>
#include <boglfw/physics/PhysicsBody.h>
#include <boglfw/utils/Event.h>
#include <boglfw/utils/bitFlags.h>

#include <atomic>

#define DEBUG_DRAW_FOOD_CHUNK

class FoodChunk: public Entity {
public:
	FoodChunk(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass);
	virtual ~FoodChunk() override;
	FunctionalityFlags getFunctionalityFlags() const override { return
			FunctionalityFlags::UPDATABLE |
			FunctionalityFlags::DRAWABLE;
	}
	static constexpr EntityType entityType = EntityType::FOOD_CHUNK;
	virtual EntityType getEntityType() const override { return entityType; }
//	glm::vec3 getWorldTransform() const override;
	aabb getAABB(bool requirePrecise=false) const override;

	void update(float dt) override;

#ifdef DEBUG_DRAW_FOOD_CHUNK
	void draw(Viewport* vp) override;
#endif

	float getInitialMass() const { return initialMass_; }
	float getMassLeft() const { return amountLeft_.load(std::memory_order_relaxed); }
	void consume(float massAmount);

	Event<void(FoodChunk*)> onDestroy;

protected:
	PhysicsBody physBody_;
	float size_;
	float initialMass_;
	std::atomic<float> amountLeft_;

private:
	static Entity* getEntityFromFoodChunkPhysBody(PhysicsBody const& body);
};

#endif /* OBJECTS_FOOD_FOODCHUNK_H_ */
