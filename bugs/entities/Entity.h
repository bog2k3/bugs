/*
 * Entity.h
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#ifndef ENTITIES_ENTITY_H_
#define ENTITIES_ENTITY_H_

#include "enttypes.h"
#include <glm/vec3.hpp>

class RenderContext;
class BinaryStream;
enum class SerializationObjectTypes;

class Entity {
public:
	virtual ~Entity();

	enum class FunctionalityFlags {
		NONE			= 0,
		DRAWABLE		= 1,
		UPDATABLE		= 2,
		SERIALIZABLE	= 4,

		ALL				= 0xFFFF,
	};

	// these flags MUST NOT change during the life time of the object, or else UNDEFINED BEHAVIOUR
	virtual FunctionalityFlags getFunctionalityFlags() { return FunctionalityFlags::NONE; }
	virtual glm::vec3 getWorldTransform() = 0;

	virtual void update(float dt) {}
	virtual void draw(RenderContext const& ctx) {}
	virtual void serialize(BinaryStream &stream);
	virtual SerializationObjectTypes getSerializationType();
	virtual EntityType getEntityType() = 0;

	void destroy();
	bool isZombie() { return markedForDeletion_; }

protected:
	Entity() = default;

private:
	bool markedForDeletion_ = false;
	bool managed_ = false;
	friend class World;
};

#endif /* ENTITIES_ENTITY_H_ */
