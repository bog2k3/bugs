/*
 * PhysicsBody.h
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#ifndef OBJECTS_PHYSICSBODY_H_
#define OBJECTS_PHYSICSBODY_H_

#include "utils/Event.h"
#include "math/box2glm.h"
#include "ObjectTypesAndFlags.h"
#include <glm/vec2.hpp>
#include <functional>

class b2Body;
class Entity;

struct PhysicsProperties {
	glm::vec2 position;
	float angle;
	bool dynamic;
	glm::vec2 velocity;
	float angularVelocity;

	PhysicsProperties(glm::vec2 position, float angle, bool dynamic, glm::vec2 velocity, float angularVelocity)
		: position(position), angle(angle), dynamic(dynamic), velocity(velocity), angularVelocity(angularVelocity)
	{}

	PhysicsProperties(glm::vec2 pos, float angle)
		: position(pos), angle(angle), dynamic(true), velocity(glm::vec2(0)), angularVelocity(0)
	{}

	PhysicsProperties() : PhysicsProperties(glm::vec2(0), 0) {}

	PhysicsProperties(const PhysicsProperties& o) = default;
};

class PhysicsBody {
public:
	PhysicsBody(ObjectTypes userObjType, void* userPtr, EventCategoryFlags::type categFlags, EventCategoryFlags::type collisionMask);
	PhysicsBody() : PhysicsBody(ObjectTypes::UNDEFINED, nullptr, 0, 0) {}
	virtual ~PhysicsBody();

	void create(PhysicsProperties const &props);
	inline glm::vec2 getPosition() { return b2g(b2Body_->GetPosition()); }
	inline Entity* getAssociatedEntity() { return getEntityFunc_(*this); }

	Event<void(PhysicsBody *other, float impulseMagnitude)> onCollision;
	Event<void(PhysicsBody* caller)> onDestroy;

	// the Box2D body:
	b2Body* b2Body_;
	// the type of object that owns this body
	ObjectTypes userObjectType_;
	// the pointer MUST be set to the object that owns this body (type of object depends on userObjectType_)
	void* userPointer_;
	// this callback MUST be set to a valid function that will return the associated entity of this body
	std::function<Entity*(PhysicsBody const& body)> getEntityFunc_;

	// bit field for the categories that this object belongs to:
	EventCategoryFlags::type categoryFlags_;
	// bit mask for what other categories of objects that this collides with should trigger onCollision events on this object
	EventCategoryFlags::type collisionEventMask_;
};

#endif /* OBJECTS_PHYSICSBODY_H_ */
