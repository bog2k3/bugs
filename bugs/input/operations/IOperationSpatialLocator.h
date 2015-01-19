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

class b2Body;

class IOperationSpatialLocator {
public:
	virtual ~IOperationSpatialLocator() {}

	virtual b2Body* getBodyAtPos(glm::vec2 pos) = 0;
	virtual void getObjectsInBox(glm::vec2 bottomLeft, glm::vec2 topRight, std::vector<WorldObject*> &outVec) = 0;
};

#endif /* INPUT_IOPERATIONSPATIALLOCATOR_H_ */
