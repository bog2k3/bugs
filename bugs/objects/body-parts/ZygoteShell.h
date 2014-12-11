/*
 * ZygoteShell.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_ZYGOTESHELL_H_
#define OBJECTS_BODY_PARTS_ZYGOTESHELL_H_

#include "BodyPart.h"

class ZygoteShell: public BodyPart {
public:
	ZygoteShell(float size, PhysicsProperties props);
	virtual ~ZygoteShell();

	void commit() override {}; // nothing to do here, the zygote does not change after creation
};

#endif /* OBJECTS_BODY_PARTS_ZYGOTESHELL_H_ */
