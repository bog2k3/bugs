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

protected:

private:
	std::vector<std::shared_ptr<GuiBasicElement>> children_;
};

#endif /* GUI_GUICONTAINERELEMENT_H_ */
