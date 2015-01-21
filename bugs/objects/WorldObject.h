/*
 * WorldObject.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef OBJECTS_WORLDOBJECT_H_
#define OBJECTS_WORLDOBJECT_H_

#include <glm/vec2.hpp>
#include <memory>

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

	PhysicsProperties() : PhysicsProperties(glm::vec2(0), 0) {}

	PhysicsProperties(const PhysicsProperties& o) = default;
};

class WorldObject : public std::enable_shared_from_this<WorldObject> {
public:
	virtual ~WorldObject();
	// creates a new world object.
	// the physics body is not yet created at this stage.
	WorldObject();

	virtual void draw(RenderContext& ctx) {}

	// creates the object's physics body from the PhysicsProperties
	void createPhysicsBody(PhysicsProperties const &props);

	b2Body* getBody() { return body_; }

	// removes this object from the world and destroys it
	void destroy();

	template<typename T>
	std::weak_ptr<T> weakThis() {
		return std::weak_ptr<T>(std::dynamic_pointer_cast<T>(shared_from_this()));
	}

protected:
	b2Body* body_;
};

#endif /* OBJECTS_WORLDOBJECT_H_ */
