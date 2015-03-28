/*
 * GuiSystem.h
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#ifndef GUI_GUISYSTEM_H_
#define GUI_GUISYSTEM_H_

#include "ICaptureManager.h"
#include <memory>
#include <vector>

class IGuiElement;
class RenderContext;
class InputEvent;

class GuiSystem : public ICaptureManager {
public:
	GuiSystem() = default;
	virtual ~GuiSystem() = default;

	void setMouseCapture(IGuiElement *elementOrNull) override {
		pCaptured = elementOrNull;
	}

	void addElement(std::shared_ptr<IGuiElement> e);
	void removeElement(std::shared_ptr<IGuiElement> e);
	void draw(RenderContext const &ctx);
	void handleInput(InputEvent &ev);

private:
	std::vector<std::shared_ptr<IGuiElement>> elements_;
	IGuiElement *pFocusedElement_ = nullptr;
	IGuiElement *pCaptured = nullptr;
	IGuiElement *lastUnderMouse = nullptr;

	IGuiElement* getElementUnderMouse(float x, float y);
	void normalizeZValuesAndSort(IGuiElement* top);
};

#endif /* GUI_GUISYSTEM_H_ */
