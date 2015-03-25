/*
 * GuiBasicElement.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIBASICELEMENT_H_
#define GUI_GUIBASICELEMENT_H_

#include "IGuiElement.h"
#include "constants.h"

class GuiBasicElement: public IGuiElement {
public:
	GuiBasicElement(glm::vec2 position, glm::vec2 size);
	virtual ~GuiBasicElement();

	void setAnchors(Anchors anch);
	glm::vec2 getPosition() { return position_; }
	void setPosition(glm::vec2 position);
	glm::vec2 getSize() { return size_; }
	void setSize(glm::vec2 size);
	void setZValue(float z);

protected:
	void getBoundingBox(glm::vec2 &outMin, glm::vec2 &outMax) override { outMin = bboxMin_; outMax = bboxMax_; }
	float getZValue() override { return zValue_; }

	virtual void mouseEnter() override;
	virtual void mouseLeave() override;
	virtual void mouseDown(MouseButtons button) override;
	virtual void mouseUp(MouseButtons button) override;
	/**
	 * override this to be informed when a valid click has occurred inside the area of the element
	 */
	virtual void clicked(glm::vec2 clickPosition) {}

private:
	glm::vec2 position_{0};
	glm::vec2 size_{0};
	glm::vec2 bboxMin_{0};
	glm::vec2 bboxMax_{0};
	float zValue_ = 0;
	Anchors anchors = Anchors::Top | Anchors::Left;

	void updateBBox();
};

#endif /* GUI_GUIBASICELEMENT_H_ */
