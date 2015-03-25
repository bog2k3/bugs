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

class GuiContainerElement: public GuiBasicElement {
public:
	GuiContainerElement(glm::vec2 position, glm::vec2 size);
	virtual ~GuiContainerElement();

	void addElement(std::shared_ptr<GuiBasicElement> e);
	void removeElement(std::shared_ptr<GuiBasicElement> e);
	void setSize(glm::vec2 size) override;

protected:

	virtual void mouseEnter() override;
	virtual void mouseLeave() override;
	virtual void mouseDown(MouseButtons button) override;
	virtual void mouseUp(MouseButtons button) override;
	virtual void mouseMoved(glm::vec2 delta, glm::vec2 position) override;
	virtual void clicked(glm::vec2 clickPosition, MouseButtons button) override;

private:
	std::vector<std::shared_ptr<GuiBasicElement>> children_;
};

#endif /* GUI_GUICONTAINERELEMENT_H_ */
