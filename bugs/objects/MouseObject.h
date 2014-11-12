/*
 * MouseObject.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bog
 */

#ifndef OBJECTS_MOUSEOBJECT_H_
#define OBJECTS_MOUSEOBJECT_H_

#include "../physics/RigidBody.h"

class MouseObject: public RigidBody {
public:
	MouseObject();
	virtual ~MouseObject();

	virtual AlignedBox getAlignedBoundingBox() const;
	virtual ArbitraryBox getOrientedBoundingBox() const;
	virtual float getMomentOfInertia() const { return 1; }
};

#endif /* OBJECTS_MOUSEOBJECT_H_ */
