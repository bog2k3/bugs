/*
 * Wall.h
 *
 *  Created on: Jan 25, 2015
 *      Author: bog
 */

#ifndef ENTITIES_WALL_H_
#define ENTITIES_WALL_H_

#include "Entity.h"
#include "enttypes.h"
#include "../physics/PhysicsBody.h"
#include "../serialization/objectTypes.h"
#include <glm/vec2.hpp>

class Wall : public Entity {
public:
	Wall(glm::vec2 const &from, glm::vec2 const &to, float width);
	virtual ~Wall();

	FunctionalityFlags getFunctionalityFlags() const override {
		return FunctionalityFlags::SERIALIZABLE;
	}

	static constexpr EntityType entityType = EntityType::WALL;
	EntityType getEntityType() const override { return entityType; }
	glm::vec3 getWorldTransform() const override;
	aabb getAABB() const override;

	SerializationObjectTypes getSerializationType() override { return SerializationObjectTypes::WALL; }
	// deserialize a Wall from the stream and add it to the world
	static void deserialize(BinaryStream &stream);
	void serialize(BinaryStream &stream) override;

protected:
	PhysicsBody body_;
	glm::vec2 from_;
	glm::vec2 to_;
	float width_;

private:
	static Entity* getEntityFromWallPhysBody(PhysicsBody const& body);
};

#endif /* ENTITIES_WALL_H_ */
