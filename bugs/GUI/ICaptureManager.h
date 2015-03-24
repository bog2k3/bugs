/*
 * ICaptureManager.h
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#ifndef GUI_ICAPTUREMANAGER_H_
#define GUI_ICAPTUREMANAGER_H_

class IGuiElement;

class ICaptureManager {
public:
	virtual ~ICaptureManager() {}

	virtual void setMouseCapture(IGuiElement *elementOrNull) = 0;
};

#endif /* GUI_ICAPTUREMANAGER_H_ */
