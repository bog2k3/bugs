/*
 * OperationPan.h
 *
 *  Created on: Nov 4, 2014
 *      Author: bog
 */

#ifndef INPUT_OPERATIONPAN_H_
#define INPUT_OPERATIONPAN_H_

#include "IOperation.h"
#include "../InputEvent.h"
#include <glm/vec2.hpp>

class OperationContext;

class OperationPan: public IOperation {
public:
	OperationPan(InputEvent::MOUSE_BUTTON assignedButton);
	virtual ~OperationPan();

	virtual void enter(const OperationContext* pContext);
	virtual void leave();
	virtual void getFocus();
	virtual void loseFocus();
	virtual void handleInputEvent(InputEvent& ev);
	virtual void update(float dt);

protected:
	const OperationContext* pContext;
	bool isFlyActive;
	bool isDragging;
	static const unsigned nFilter = 5;
	glm::vec2 lastDelta[nFilter];
	float filterTimes[nFilter];
	int lastIndex;
	float frictionFactor;
	glm::vec2 flySpeed;
	InputEvent::MOUSE_BUTTON boundButton;
};

#endif /* INPUT_OPERATIONPAN_H_ */
