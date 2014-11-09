#include "stdafx.h"
#include "SelectOperation.h"
#include "OperationContext.h"
#include "../lifeApp.h"

namespace lifeApplication
{
const DWORD SelectOperation::SELECTION_COLOR = 0xFFFFD020;

	SelectOperation::SelectOperation(vector<WorldObject*> &vecSelectionOutput)
		: isSelecting(false)
		, vecSelectionOutput(vecSelectionOutput)
	{
		ZeroMemory(&mouseDownLocation, sizeof(mouseDownLocation));
		ZeroMemory(&mouseCurrentLocation, sizeof(mouseCurrentLocation));
	}

	SelectOperation::~SelectOperation()
	{
	}

	void SelectOperation::enter(const OperationContext* pContext)
	{
		this->pContext = pContext;
		pSelectionRectOSD = new Render::Rectangle(Vector2());
		pContext->pRenderer->registerRenderable(pSelectionRectOSD);
	}

	void SelectOperation::leave()
	{
		pContext->pRenderer->unregisterRenderable(pSelectionRectOSD);
		delete pSelectionRectOSD;
	}

	void SelectOperation::activate()
	{
	}

	void SelectOperation::deactivate()
	{
	}

	bool SelectOperation::mouseMoved(int x, int y, int deltaX, int deltaY, bool leftBtnDown, bool rightBtnDown, int fwKeys)
	{
		mouseCurrentLocation.x = x;
		mouseCurrentLocation.y = y;
		if (isSelecting) {
			updateSelection(x,y);
			return true;
		} else
			return false;
	}

	bool SelectOperation::mouseDown(MouseButton button, int x, int y, int fwKeys)
	{
		mouseDownLocation.x = x;
		mouseDownLocation.y = y;
		mouseCurrentLocation = mouseDownLocation;
		if (button == MOUSE_LEFTBTN) {
			beginSelect(x, y);
			return true;
		} else {
			endSelect();
			return false;
		}
	}

	bool SelectOperation::mouseUp(MouseButton button, int x, int y, int fwKeys)
	{
		if (button == MOUSE_LEFTBTN) {
			updateSelection(x, y);
			endSelect();
			return true;
		} else
			return false;
	}

	void SelectOperation::beginSelect(int x, int y)
	{
		isSelecting = true;
		updateSelection(x, y);
		pContext->pViewport->addPermanentOSDElement(pSelectionRectOSD, Render::ANCHOR_LEFTTOP, 0, 0);
	}

	void SelectOperation::endSelect()
	{
		isSelecting = false;
		pContext->pViewport->removePermanentOSDElement(pSelectionRectOSD);
	}

	void SelectOperation::updateSelection(int newMouseX, int newMouseY)
	{
		Render::IViewport* pViewport = pContext->pViewport;
		Vector2 v1(min(newMouseX, mouseDownLocation.x), max(newMouseY, mouseDownLocation.y)); // bottom-left
		Vector2 v2(max(newMouseX, mouseDownLocation.x), min(newMouseY, mouseDownLocation.y)); // top-right
		RECT vpRect; pViewport->getScreenRect(&vpRect);
		v1.x -= vpRect.left;
		v1.y -= vpRect.top;
		v2.x -= vpRect.left;
		v2.y -= vpRect.top;
		pViewport->unproject(&v1);
		pViewport->unproject(&v2);

		pContext->pApplication->suspendRendering();
		vecSelectionOutput.clear();
		pContext->pApplication->pauseSimulation();
		pContext->pBSPTree->retrieveWorldObjectsInBox(AlignedBox(v1,v2), &vecSelectionOutput);
		pContext->pApplication->resumeSimulation();
		if (isSelecting) {
			// draw selection rectangle
			AlignedBox boxSel;
			boxSel.bottomLeft.x = min(mouseCurrentLocation.x, mouseDownLocation.x);
			boxSel.bottomLeft.y = max(mouseCurrentLocation.y, mouseDownLocation.y);
			boxSel.topRight.x = max(mouseCurrentLocation.x, mouseDownLocation.x);
			boxSel.topRight.y = min(mouseCurrentLocation.y, mouseDownLocation.y);
			pSelectionRectOSD->fromBox(boxSel); pSelectionRectOSD->lineColor = SELECTION_COLOR;
		}
		pContext->pApplication->resumeRendering();
	}

}