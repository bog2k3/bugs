/*
 * IGuiElement.h
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#ifndef GUI_IGUIELEMENT_H_
#define GUI_IGUIELEMENT_H_

#include "constants.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class RenderContext;
class ICaptureManager;

class IGuiElement {
public:
	virtual ~IGuiElement() {}

	virtual void getBoundingBox(glm::vec2 &outMin, glm::vec2 &outMax) = 0;
	virtual float getZValue() = 0;
	virtual void setZValue(float z) = 0;

	void setCaptureManager(ICaptureManager* mgr) { captureManager_ = mgr; }
	ICaptureManager* getCaptureManager() { return captureManager_; }

protected:
	friend class GuiSystem;

	virtual void draw(RenderContext const &ctx, glm::vec3 frameTranslation, glm::vec2 frameScale) = 0;

	virtual void mouseEnter() {}
	virtual void mouseLeave() {}
	virtual void mouseDown(MouseButtons button) {}
	virtual void mouseUp(MouseButtons button) {}
	virtual void mouseMoved(glm::vec2 delta, glm::vec2 position) {}
	virtual void mouseScroll(float delta) {}
	virtual void keyDown(int keyCode) {}
	virtual void keyUp(int keyCode) {}
	virtual void keyChar(char c) {}

	virtual void focusGot() {}
	virtual void focusLost() {}

private:
	ICaptureManager *captureManager_ = nullptr;
};

#endif /* GUI_IGUIELEMENT_H_ */
