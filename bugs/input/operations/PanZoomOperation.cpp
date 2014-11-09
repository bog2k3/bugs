#include "stdafx.h"
#include "PanZoomOperation.h"
#include "OperationContext.h"
#include "../Render/IViewport.h"
#include "../lifeApp.h"

namespace lifeApplication
{
	PanZoomOperation::PanZoomOperation()
		: pContext(NULL)
		, isPanning(false)
		, isPanningAlt(false)
	{
	}

	PanZoomOperation::~PanZoomOperation()
	{
	}

	void PanZoomOperation::enter(const OperationContext* pContext)
	{
		this->pContext = pContext;
	}

	void PanZoomOperation::leave()
	{
	}

	void PanZoomOperation::activate()
	{
	}

	void PanZoomOperation::deactivate()
	{
	}

	bool PanZoomOperation::mouseWheel(int x, int y, float zDelta, int fwKeys)
	{
		Render::IViewport* viewport = pContext->pViewport;
		double scale = viewport->getScale();

		float zoomSpeed = (fwKeys & MK_CONTROL) != 0 ? 1.f : 0.1f;

		if (zDelta < 0)
			scale *= 1 - zDelta * zoomSpeed; // zoom out
		else
			scale /= 1 + zDelta * zoomSpeed; // zoom in

		viewport->setScale(scale);

		return true;
	}

	bool PanZoomOperation::mouseMoved(int x, int y, int deltaX, int deltaY, bool leftBtnDown, bool rightBtnDown, int fwKeys)
	{
		if (isPanning || (isPanningAlt && (fwKeys & MK_CONTROL))) {
			pContext->pViewport->pan(deltaX, deltaY);
			return true;
		} else
			return false;
	}

	bool PanZoomOperation::mouseDown(MouseButton button, int x, int y, int fwKeys)
	{
		if (isPanning || isPanningAlt) {
			isPanning = false;
			isPanningAlt = false;
			return true;
		}
		isPanning = (button == MOUSE_MIDDLEBTN);
		isPanningAlt = button == MOUSE_LEFTBTN && (fwKeys & MK_CONTROL);
		return isPanning || isPanningAlt;
	}

	bool PanZoomOperation::mouseUp(MouseButton button, int x, int y, int fwKeys)
	{
		if ((isPanning && button == MOUSE_MIDDLEBTN) || (isPanningAlt && button == MOUSE_LEFTBTN)) {
			isPanning = false;
			isPanningAlt = false;
			return true;
		} else
			return false;
	}

	bool PanZoomOperation::keyDown(int vKeyCode)
	{
		switch (vKeyCode) {
			case VK_OEM_PLUS:
				if (GetKeyState(VK_CONTROL) & 0x80) {
					mouseWheel(0, 0, +1, 0);
					return true;
				} else
					return false;
			case VK_OEM_MINUS:
				if (GetKeyState(VK_CONTROL) & 0x80) {
					mouseWheel(0, 0, -1, 0);
					return true;
				} else
					return false;
			default:
				return false;
		};
	}

}