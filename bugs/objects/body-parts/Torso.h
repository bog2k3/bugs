/*
 * Torso.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_TORSO_H_
#define OBJECTS_BODY_PARTS_TORSO_H_

#include "BodyPart.h"

class Torso : public BodyPart {
public:
	Torso(BodyPart* parent, PhysicsProperties props);
	~Torso() override;

	void commit() override;
	void draw(ObjectRenderContext* ctx) override;

	// returns the 'size' (surface area)
	float getSize() { return size_; }
	float getDensity() { return density_; }

	void setSize(float val);
	void setDensity(float val);

protected:
	float size_;
	float density_;
};

#endif /* OBJECTS_BODY_PARTS_TORSO_H_ */
