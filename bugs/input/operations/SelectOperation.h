#pragma once
#include "IOperation.h"
#include "../Render/OSD/Rectangle.h"

namespace lifeApplication
{
	class WorldObject;

	class SelectOperation : public IOperation
	{
	public:
		SelectOperation(vector<WorldObject*> &vecSelectionOutput);
		virtual ~SelectOperation();

		virtual void enter(const OperationContext* pContext);
		virtual void leave();
		virtual void activate();
		virtual void deactivate();
		virtual bool mouseMoved(int x, int y, int deltaX, int deltaY, bool leftBtnDown, bool rightBtnDown, int fwKeys);
		virtual bool mouseDown(MouseButton button, int x, int y, int fwKeys);
		virtual bool mouseUp(MouseButton button, int x, int y, int fwKeys);

		static const DWORD SELECTION_COLOR;

	protected:
		const OperationContext *pContext;
		bool isSelecting;
		vector<WorldObject*> &vecSelectionOutput;
		POINT mouseDownLocation;
		POINT mouseCurrentLocation;
		Render::Rectangle* pSelectionRectOSD;

		void updateSelection(int newMouseX, int newMouseY);
		void beginSelect(int x, int y);
		void endSelect();
	};
}