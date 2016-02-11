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
#include "../PhysicsBody.h"
#include "../serialization/objectTypes.h"
#include <glm/vec2.hpp>

class Wall : public Entity {
public:
	Wall(glm::vec2 const &from, glm::vec2 const &to, float width);
	virtual ~Wall();

	FunctionalityFlags getFunctionalityFlags() override {
		return FF_SERIALIZABLE;
	}

	static constexpr EntityType::Values entityType = EntityType::WALL;
	virtual EntityType::Values getEntityType() override { return entityType; }

	SerializationObjectTypes getSerializationType() override { return SerializationObjectTypes::WALL; }
	// deserialize a Wall from the stream and add it to the world
	static void deserialize(BinaryStream &stream);
	void serialize(BinaryStream &stream) override;

protected:
	PhysicsBody body_;
	glm::vec2 from_;
	glm::vec2 to_;
	float width_;
};

#endif /* ENTITIES_WALL_H_ */
