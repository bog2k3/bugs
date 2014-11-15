/*
 * IOperationSpatialLocator.h
 *
 *  Created on: Nov 15, 2014
 *      Author: bog
 */

#ifndef INPUT_IOPERATIONSPATIALLOCATOR_H_
#define INPUT_IOPERATIONSPATIALLOCATOR_H_

#include <glm/fwd.hpp>
#include <vector>

class WorldObject;
class AlignedBox;

class IOperationSpatialLocator {
public:
	virtual ~IOperationSpatialLocator() {}

	virtual WorldObject* getObjectAtPos(glm::vec2 pos) = 0;
	virtual void getObjectsInBox(AlignedBox box, std::vector<WorldObject*> &outVec) = 0;
};

#endif /* INPUT_IOPERATIONSPATIALLOCATOR_H_ */
