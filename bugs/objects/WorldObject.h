/*
 * WorldObject.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef OBJECTS_WORLDOBJECT_H_
#define OBJECTS_WORLDOBJECT_H_

#include <glm/vec2.hpp>

// render classes:
class RenderContext;
// physics classes:
class b2Body;

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

	PhysicsProperties(const PhysicsProperties& o) = default;
};

class WorldObject {
public:
	virtual ~WorldObject();
	// creates a new world object with the initial properties props.
	// the physics body is not yet created at this stage unless autoCommit is set to true. This allows
	// more tweaking to be done to the WorldObject's physicsProperties before the body is finally created by calling commit().
	WorldObject(PhysicsProperties props, bool autoCreatePhysicsBody=false);

	virtual void draw(RenderContext* ctx) {}

	// creates the object's physics body from the object's PhysicsProperties
	void createPhysicsBody();

	b2Body* getBody() { return body_; }
	PhysicsProperties& getPhysicsProp() { return *initialData_; }

protected:
	b2Body* body_;
	PhysicsProperties *initialData_;	// these are valid only for the initial state of the object
	void purgeInitializationData();
};

#endif /* OBJECTS_WORLDOBJECT_H_ */
