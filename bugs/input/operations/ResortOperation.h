#pragma once
#include "../../math/math3D.h"
#include "IOperation.h"

namespace Render { class Rectangle; class Line; };

namespace lifeApplication
{
	class WorldObject;

	class ResortOperation : public IOperation
	{
	public:

		ResortOperation(HINSTANCE hInstance);
		virtual ~ResortOperation();

		virtual void enter(const OperationContext* pContext);
		virtual void leave();
		virtual void activate();
		virtual void deactivate();
		virtual bool mouseDown(MouseButton button, int x, int y, int fwKeys);
		virtual bool mouseUp(MouseButton button, int x, int y, int fwKeys);
		virtual bool command(int cmd, LPARAM lParam);

	protected:
		const OperationContext* pContext;
		bool isConsumed;
		HINSTANCE hInstance;
		HMENU m_popupMenu;
		HMENU m_popupMenuRoot;

		class ResortAttachEnd : public IOperation
		{
		public:
			ResortAttachEnd(int which_end, WorldObject* pFirstObj, Vector2 &vFirst, double length);
			virtual ~ResortAttachEnd() { }

			virtual void enter(const OperationContext* pContext);
			virtual bool mouseMoved(int x, int y, int deltaX, int deltaY, 
				bool leftBtnDown, bool rightBtnDown, int fwKeys);
			virtual bool mouseWheel(int x, int y, float zDelta, int fwKeys);
			virtual bool mouseDown(MouseButton button, int x, int y, int fwKeys);
			virtual bool mouseUp(MouseButton button, int x, int y, int fwKeys);
			virtual void leave();
			virtual void activate();
			virtual void deactivate();
		protected:
			int which_end;
			const OperationContext* pContext;
			int msgID;
			WorldObject* pFirstObj;
			Vector2 vFirstPoint;
			vector<WorldObject*> vecObjectsUnderMouse;
			WorldObject* pCurrentObj;
			Render::Rectangle* pSelectionRect[2];
			bool selectionRectVisible[2];
			bool clicked;

			void updateSelectionRect();
		};

		class ResortSetLength : public IOperation
		{
		public:

			ResortSetLength(Vector2 &vFirstEnd, WorldObject* pFirstObj);
			virtual ~ResortSetLength(){}

			virtual void enter(const OperationContext* pContext);
			virtual void leave();
			virtual void activate();
			virtual void deactivate();
			virtual bool mouseMoved(int x, int y, int deltaX, int deltaY, 
				bool leftBtnDown, bool rightBtnDown, int fwKeys);
			virtual bool mouseDown(MouseButton button, int x, int y, int fwKeys);
			virtual bool mouseUp(MouseButton button, int x, int y, int fwKeys);
		protected:
			const OperationContext* pContext;
			int msgID;
			WorldObject* pFirstObject;
			Render::Rectangle* pSelectionRect;
			Render::Line* pResortLine;
			Vector2 vFirstEnd;
			bool clicked;
		};
	};


}