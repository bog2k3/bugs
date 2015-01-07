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
	ZygoteShell(float size);
	~ZygoteShell() override;

	void commit() override;
	void draw(RenderContext& ctx) override;

	/**
	 * this updates the BodyPartInitializationData::cachedProps from the actual zygot physics body,
	 * in order for these to be inherited by all other body parts
	 */
	void updateCachedDynamicPropsFromBody();

private:
};

#endif /* OBJECTS_BODY_PARTS_ZYGOTESHELL_H_ */
