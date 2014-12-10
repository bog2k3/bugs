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
	WorldObject(PhysicsProperties props);

	virtual void draw(ObjectRenderContext* ctx) {}

	b2Body* getBody() { return body_; }

protected:
	b2Body* body_;
	PhysicsProperties physProps_;
};

#endif /* OBJECTS_WORLDOBJECT_H_ */
