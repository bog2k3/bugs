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
	ZygoteShell(glm::vec2 position, glm::vec2 velocity, float mass);
	~ZygoteShell() override;

	void draw(RenderContext const& ctx) override;

	/**
	 * this updates the BodyPartInitializationData::cachedProps from the actual zygot physics body,
	 * in order for these to be inherited by all other body parts
	 */
	void updateCachedDynamicPropsFromBody();

	float getMass() { return mass_; }
	//int getDepth() override { return -1; }

protected:
	void commit() override;
	void die() override;

private:
	float mass_;
	bool dead_;
};

#endif /* OBJECTS_BODY_PARTS_ZYGOTESHELL_H_ */
