/*
 * Entity.h
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#ifndef ENTITIES_ENTITY_H_
#define ENTITIES_ENTITY_H_

#include "enttypes.h"
#include "../utils/bitFlags.h"

#include <glm/vec3.hpp>
#include <atomic>

class RenderContext;
class BinaryStream;
enum class SerializationObjectTypes;
struct aabb;

class Entity {
public:
	virtual ~Entity();

	enum class FunctionalityFlags {
		NONE			= 0,
		DONT_CARE		= 0,
		DRAWABLE		= 1,
		UPDATABLE		= 2,
		SERIALIZABLE	= 4,
	};

	// these flags MUST NOT change during the life time of the object, or else UNDEFINED BEHAVIOUR
	virtual FunctionalityFlags getFunctionalityFlags() const { return FunctionalityFlags::NONE; }
	virtual glm::vec3 getWorldTransform() const = 0;
	glm::vec2 getPosition();

	virtual void update(float dt) {}
	virtual void draw(RenderContext const& ctx) {}
	virtual void serialize(BinaryStream &stream);
	virtual SerializationObjectTypes getSerializationType();
	virtual EntityType getEntityType() const = 0;
	virtual aabb getAABB() const = 0;

	void destroy();
	bool isZombie() const { return markedForDeletion_.load(std::memory_order_acquire); }

protected:
	Entity() = default;

private:
	std::atomic<bool> markedForDeletion_ {false};
	bool managed_ = false;
	friend class World;
};

#endif /* ENTITIES_ENTITY_H_ */
