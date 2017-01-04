/*
 * PhysContactListener.h
 *
 *  Created on: Jan 22, 2015
 *      Author: bogdan
 */

#ifndef PHYSCONTACTLISTENER_H_
#define PHYSCONTACTLISTENER_H_

#include "../utils/MTVector.h"
#include <Box2D/Dynamics/b2WorldCallbacks.h>

class PhysicsBody;

class PhysContactListener : public b2ContactListener {
public:
	PhysContactListener();
	virtual ~PhysContactListener();

	// void BeginContact(b2Contact* contact) override;
	// void EndContact(b2Contact* contact) override;
	// void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

	void update(float dt);

private:
	struct eventData {
		PhysicsBody* target;
		PhysicsBody* argument;
		float impulseMagnitude;

		eventData(PhysicsBody* target, PhysicsBody* arg, float imp)
			: target(target), argument(arg), impulseMagnitude(imp) {
		}
	};
	MTVector<eventData> eventBuffer;
};

#endif /* PHYSCONTACTLISTENER_H_ */
