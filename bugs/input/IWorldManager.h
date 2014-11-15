/*
 * IWorldManager.h
 *
 *  Created on: Nov 15, 2014
 *      Author: bog
 */

#ifndef INPUT_IWORLDMANAGER_H_
#define INPUT_IWORLDMANAGER_H_

class WorldObject;

class IWorldManager {
public:
	virtual ~IWorldManager() {}

	virtual void addObject(WorldObject* obj) = 0;
	virtual void removeObject(WorldObject* obj) = 0;
};

#endif /* INPUT_IWORLDMANAGER_H_ */
