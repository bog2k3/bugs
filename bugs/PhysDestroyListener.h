/*
 * PhysDestroyListener.h
 *
 *  Created on: Mar 22, 2015
 *      Author: bog
 */

#ifndef PHYSDESTROYLISTENER_H_
#define PHYSDESTROYLISTENER_H_

#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <map>
#include <vector>
#include <functional>

class PhysDestroyListener : public b2DestructionListener
{
public:
	PhysDestroyListener() {}
	~PhysDestroyListener() {}

	/// Called when any joint is about to be destroyed due
	/// to the destruction of one of its attached bodies.
	void SayGoodbye(b2Joint* joint) override;

	/// Called when any fixture is about to be destroyed due
	/// to the destruction of its parent body.
	void SayGoodbye(b2Fixture* fixture) override {}

	typedef std::function<void(b2Joint* joint)> callbackType;
	unsigned addCallback(b2Joint* joint, callbackType cb);
	void removeCallback(b2Joint* joint, unsigned handle);

private:
	std::map<b2Joint*, std::vector<callbackType>> mapCallbacks;
};

#endif /* PHYSDESTROYLISTENER_H_ */
