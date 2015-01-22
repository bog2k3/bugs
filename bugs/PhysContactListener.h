/*
 * PhysContactListener.h
 *
 *  Created on: Jan 22, 2015
 *      Author: bogdan
 */

#ifndef PHYSCONTACTLISTENER_H_
#define PHYSCONTACTLISTENER_H_

#include <Box2D/Dynamics/b2WorldCallbacks.h>

class PhysContactListener : public b2ContactListener {
public:
	PhysContactListener();
	virtual ~PhysContactListener();

	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
};

#endif /* PHYSCONTACTLISTENER_H_ */
