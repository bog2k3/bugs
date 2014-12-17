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
class Shape2D;
class Viewport;
// physics classes:
class b2Body;

class ObjectRenderContext {
public:
	Shape2D* shape;
	Viewport* viewport;

	ObjectRenderContext(Shape2D* shape, Viewport* vp)
		: shape(shape), viewport(vp) {
	}

	ObjectRenderContext()
		: shape(nullptr), viewport(nullptr) {
	}
};

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
	WorldObject(PhysicsProperties props, bool autoCommit=false);

	virtual void draw(ObjectRenderContext* ctx) {}

	// creates the object's physics body from the object's PhysicsProperties
	void commit();

	b2Body* getBody() { return body_; }
	PhysicsProperties& getPhysicsProp() { return *physProps_; }

protected:
	b2Body* body_;
	PhysicsProperties *physProps_;	// these are valid only for the initial state of the object

private:
	bool committed_;
};

#endif /* OBJECTS_WORLDOBJECT_H_ */
