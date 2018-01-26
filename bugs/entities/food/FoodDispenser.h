/*
 * FoodDispenser.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_FOOD_FOODDISPENSER_H_
#define OBJECTS_FOOD_FOODDISPENSER_H_

#include "../enttypes.h"
#include "../../serialization/objectTypes.h"

#include <boglfw/entities/Entity.h>
#include <boglfw/physics/PhysicsBody.h>
#include <boglfw/utils/bitFlags.h>

class FoodDispenser: public Entity {
public:
	FoodDispenser(glm::vec2 const &position, float direction);
	virtual ~FoodDispenser();

	static constexpr EntityType entityType = EntityType::FOOD_DISPENSER;
	virtual EntityType getEntityType() const override { return entityType; }
	glm::vec3 getWorldTransform() const override;
	aabb getAABB() const override;

	FunctionalityFlags getFunctionalityFlags() const override { return
			FunctionalityFlags::UPDATABLE |
			FunctionalityFlags::SERIALIZABLE;
	}

	int getSerializationType() const override { return SerializationObjectTypes::FOOD_DISPENSER; }
	// deserialize a dispenser from the stream and add it to the world
	static void deserialize(BinaryStream &stream);
	void serialize(BinaryStream &stream) const override;


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

private:
	static Entity* getEntityFromFoodDispenserPhysBody(PhysicsBody const& body);
};

#endif /* OBJECTS_FOOD_FOODDISPENSER_H_ */

