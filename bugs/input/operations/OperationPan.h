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
	virtual ~OperationPan() override;

	void enter(const OperationContext* pContext) override;
	void leave() override;
	void getFocus() override;
	void loseFocus() override;
	void handleInputEvent(InputEvent& ev) override;
	void update(float dt) override;

protected:
	const OperationContext* pContext;
	bool isFlyActive;
	bool isDragging;
	static constexpr unsigned nFilter = 5;
	glm::vec2 lastDelta[nFilter];
	float filterTimes[nFilter];
	int lastIndex;
	float frictionFactor;
	glm::vec2 flySpeed;
	InputEvent::MOUSE_BUTTON boundButton;
};

#endif /* INPUT_OPERATIONPAN_H_ */
