/*
 * GuiContainerElement.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUICONTAINERELEMENT_H_
#define GUI_GUICONTAINERELEMENT_H_

#include "GuiBasicElement.h"
#include <vector>
#include <memory>

class GuiContainerElement: public GuiBasicElement {
public:
	GuiContainerElement(glm::vec2 position, glm::vec2 size);
	virtual ~GuiContainerElement();

	void addElement(std::shared_ptr<GuiBasicElement> e);
	void removeElement(std::shared_ptr<GuiBasicElement> e);
	void setSize(glm::vec2 size) override;
	std::shared_ptr<GuiBasicElement> getPointedElement() { return elementUnderMouse_; }

protected:

#warning "must change coordinate space for draw and mouse events"
#warning "all coordinates must be in parent-space. Add support in shape2D for translation stack to simplify drawing code"
	virtual void draw(RenderContext const &ctx, glm::vec3 frameTranslation, glm::vec2 frameScale) override;
	virtual void mouseDown(MouseButtons button) override;
	virtual void mouseUp(MouseButtons button) override;
	virtual void mouseMoved(glm::vec2 delta, glm::vec2 position) override;
	virtual void clicked(glm::vec2 clickPosition, MouseButtons button) override;
	virtual void keyDown(int keyCode) override;
	virtual void keyUp(int keyCode) override;
	virtual void keyChar(char c) override;

	void setClientArea(glm::vec2 offset, glm::vec2 counterOffset);
	void getClientArea(glm::vec2 &outOffset, glm::vec2 &outSize);

private:
	glm::vec2 clientAreaOffset_{0};	// (positive) offset from top left corner of container to top-left corner of client area
	glm::vec2 clientAreaCounterOffset_{0}; // (positive) offset from bottom-right corner of client area to corner of container
	glm::vec2 clientAreaSize_{0};
	std::vector<std::shared_ptr<GuiBasicElement>> children_;
	std::shared_ptr<GuiBasicElement> elementUnderMouse_ = nullptr;
	std::shared_ptr<GuiBasicElement> focusedElement_ = nullptr;

	void updateClientArea();
};

#endif /* GUI_GUICONTAINERELEMENT_H_ */
