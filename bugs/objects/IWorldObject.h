/*
 * IWorldObject.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef OBJECTS_IWORLDOBJECT_H_
#define OBJECTS_IWORLDOBJECT_H_

// render objects:
class Rectangle;
// end render objects

struct ObjectRenderContext {
	Rectangle* rectangle;
};

class IWorldObject {
public:
	virtual ~IWorldObject() {}
	virtual void draw(ObjectRenderContext*) = 0;
};

#endif /* OBJECTS_IWORLDOBJECT_H_ */
