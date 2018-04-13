/*
 * ZygoteShell.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_ZYGOTESHELL_H_
#define OBJECTS_BODY_PARTS_ZYGOTESHELL_H_

#include "BodyPart.h"

#include <glm/vec2.hpp>

class ZygoteShell: public BodyPart {
public:
	ZygoteShell(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass, BodyPartContext const& context);
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
	void updateFixtures() override;

private:
	float mass_;
};

#endif /* OBJECTS_BODY_PARTS_ZYGOTESHELL_H_ */
