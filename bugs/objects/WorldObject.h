/*
 * WorldObject.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef OBJECTS_WORLDOBJECT_H_
#define OBJECTS_WORLDOBJECT_H_

#include <glm/fwd.hpp>

// render classes:
class Shape2D;
class Viewport;
// end render classes

// physics classes:
class b2World;
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

class WorldObject {
public:
	virtual ~WorldObject();
	WorldObject(b2World* world, glm::vec2 position, float angle, bool dynamic, glm::vec2 velocity, float angularVelocity);

	virtual void draw(ObjectRenderContext* ctx);

	b2Body* getBody() { return body; }

protected:
	b2Body* body;
};

#endif /* OBJECTS_WORLDOBJECT_H_ */
