/*
 * OperationPan.h
 *
 *  Created on: Nov 4, 2014
 *      Author: bog
 */

#ifndef INPUT_OPERATIONPAN_H_
#define INPUT_OPERATIONPAN_H_

#include "IOperation.h"
#include <glm/vec2.hpp>

class IRenderer;

class OperationPan: public IOperation {
public:
	explicit OperationPan(IRenderer* renderer);
	virtual ~OperationPan();

	virtual void handleInput(InputEvent& ev);
	virtual void update(float dt);

protected:
	IRenderer* pRenderer;
	bool isFlyActive;
	bool isDragging;
	static const unsigned nFilter = 5;
	glm::vec2 lastDelta[nFilter];
	int lastIndex;
	float frictionFactor;
	glm::vec2 flySpeed;
};

#endif /* INPUT_OPERATIONPAN_H_ */
