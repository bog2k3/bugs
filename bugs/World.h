/*
 * World.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "objects/WorldObject.h"
#include "input/operations/IOperationSpatialLocator.h"
#include "input/IWorldManager.h"
#include <vector>
#include <list>

class World : public IOperationSpatialLocator, public IWorldManager {
public:
	World();
	virtual ~World();

	WorldObject* getObjectAtPos(glm::vec2 pos);
	void getObjectsInBox(AlignedBox box, std::vector<WorldObject*> &outVec);

	void setRenderContext(ObjectRenderContext ctxt) { renderContext = ctxt; }
	void draw();

	void addObject(WorldObject* obj);
	void removeObject(WorldObject* obj);

protected:
	std::list<WorldObject*> objects;
	ObjectRenderContext renderContext;
};

#endif /* WORLD_H_ */
