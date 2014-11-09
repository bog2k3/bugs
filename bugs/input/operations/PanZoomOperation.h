#pragma once
#include "IOperation.h"

namespace lifeApplication
{
	class PanZoomOperation : public IOperation
	{
	public:
		PanZoomOperation();
		virtual ~PanZoomOperation();

		virtual void enter(const OperationContext* pContext);
		virtual void leave();
		virtual void activate();
		virtual void deactivate();
		virtual bool mouseWheel(int x, int y, float zDelta, int fwKeys);
		virtual bool mouseMoved(int x, int y, int deltaX, int deltaY, bool leftBtnDown, bool rightBtnDown, int fwKeys);
		virtual bool mouseDown(MouseButton button, int x, int y, int fwKeys);
		virtual bool mouseUp(MouseButton button, int x, int y, int fwKeys);
		virtual bool keyDown(int vKeyCode);

	protected:
		const OperationContext *pContext;
		bool isPanning; // panning with middleMouse
		bool isPanningAlt; // alternative panning with CTRL+leftMouse
	};
}