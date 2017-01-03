/*
 * FoodDispenser.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef OBJECTS_FOOD_FOODDISPENSER_H_
#define OBJECTS_FOOD_FOODDISPENSER_H_

#include "../Entity.h"
#include "../enttypes.h"
#include "../../physics/PhysicsBody.h"
#include "../../serialization/objectTypes.h"
#include "../../utils/bitFlags.h"

class FoodDispenser: public Entity {
public:
	FoodDispenser(glm::vec2 const &position, float direction);
	virtual ~FoodDispenser();

	static constexpr EntityType entityType = EntityType::FOOD_DISPENSER;
	virtual EntityType getEntityType() override { return entityType; }
	glm::vec3 getWorldTransform() const override;
	aabb getAABB() const override;

	FunctionalityFlags getFunctionalityFlags() override { return
			FunctionalityFlags::UPDATABLE |
			FunctionalityFlags::SERIALIZABLE;
	}

	SerializationObjectTypes getSerializationType() override { return SerializationObjectTypes::FOOD_DISPENSER; }
	// deserialize a dispenser from the stream and add it to the world
	static void deserialize(BinaryStream &stream);
	void serialize(BinaryStream &stream) override;


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

