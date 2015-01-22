/*
 * PhysicsBody.h
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#ifndef OBJECTS_PHYSICSBODY_H_
#define OBJECTS_PHYSICSBODY_H_

#include <glm/vec2.hpp>

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

class PhysicsBody {
public:
	PhysicsBody() : b2Body_(nullptr) {}
	virtual ~PhysicsBody();

	void create(PhysicsProperties const &props);

	b2Body* b2Body_;
};

#endif /* OBJECTS_PHYSICSBODY_H_ */
