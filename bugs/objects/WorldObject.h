/*
 * WorldObject.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef OBJECTS_WORLDOBJECT_H_
#define OBJECTS_WORLDOBJECT_H_

// render objects:
class Shape2D;
class Viewport;
// end render objects

class RigidBody;
class Spring;

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

enum WorldObjectType {
	TYPE_RIGID,
	TYPE_SPRING,
};

class WorldObject {
public:
	virtual ~WorldObject();
	WorldObject(RigidBody* rigid); // this makes this object owner of the rigid body passed (will delete it on destructor)
	WorldObject(Spring* spring); // this makes this object owner of the rigid body passed (will delete it on destructor)
	WorldObject(WorldObjectType type); // ownership of physics objects falls on descendant class (responsibility to delete)

	virtual void draw(ObjectRenderContext* ctx);

	virtual RigidBody* getRigidBody() const { return rigidBody; }
	virtual Spring* getSpring() const { return spring; }
	WorldObjectType getType() const { return type; }

protected:
	WorldObjectType const type;
	RigidBody* const rigidBody;
	Spring* const spring;

private:
	bool ownerOfResource;

};

#endif /* OBJECTS_WORLDOBJECT_H_ */
