/*
 * BonePhysicsComponent.h
 *
 *  Created on: Nov 13, 2014
 *      Author: bog
 */

#include "../../physics/RigidBody.h"

class BonePhysicsComponent : public RigidBody {
public:
	virtual AlignedBox getAlignedBoundingBox() const {
		return AlignedBox(0,0,0,0);
	}
	virtual ArbitraryBox getOrientedBoundingBox() const {
		return ArbitraryBox::fromAlignedBox(getLocalBoundingBox(), getRotation());
	}
	virtual AlignedBox getLocalBoundingBox() const {
		return AlignedBox(-size*0.5f, size*0.5f);
	}

	virtual float getMomentOfInertia() const {
		// moment of inertia for a rectangular object:
		return 1.f/12.f * getMass() * (size.x*size.x + size.y*size.y);
	}

	BonePhysicsComponent(float mass, glm::vec2 position, float rotation, glm::vec2 size,
						 glm::vec2 initialVelocity, float initialAngularVelocity)
		: RigidBody(mass, position, rotation, initialVelocity, initialAngularVelocity)
		, size(size) {
	}

protected:
	glm::vec2 size;
};
