#include "stdafx.h"
#include "../resource.h"
#include "ResortOperation.h"
#include "OperationContext.h"
#include "OperationsStack.h"
#include "../Render/OSD/IOSDTextManager.h"
#include "../Render/OSD/Line.h"
#include "../Render/OSD/Rectangle.h"
#include "../Render/IViewport.h"
#include "../lifeApp.h"

namespace lifeApplication
{
/// attach end sub-operation ----------------------------------------------
#pragma warning(disable:4351) // "warning C4351: new behavior: elements of array will be default initialized"

	ResortOperation::ResortAttachEnd::ResortAttachEnd(int which_end, WorldObject* pFirstObj, Vector2 &vFirst, 
		double length)
		: pSelectionRect() 
		, msgID(-1)
		, vecObjectsUnderMouse()
		, pContext(NULL)
		, which_end(which_end)
		, selectionRectVisible()
		, pCurrentObj(NULL)
		, pFirstObj(pFirstObj)
		, clicked(false)
	{
		assert(which_end == 1 || (which_end == 2 && pFirstObj!=NULL));
	}

	void ResortOperation::ResortAttachEnd::enter(const OperationContext* pContext) 
	{
		this->pContext = pContext;
		for (int i=0; i<2; ++i) {
			pSelectionRect[i] = new Render::Rectangle(Vector2());
			pContext->pRenderer->registerRenderable(pSelectionRect[i]);
			pSelectionRect[i]->lineColor = 0xFF00FF00;
			if (which_end == 1)
				break;
		}
		if (which_end == 2) {
			// first selection rectangle must always be visible
			AlignedBox bbox; pFirstObj->getBoundingBox(&bbox);
			pContext->pViewport->project(&bbox.bottomLeft);
			pContext->pViewport->project(&bbox.topRight);
			pSelectionRect[0]->fromBox(bbox);
		}
	}

	bool ResortOperation::ResortAttachEnd::mouseMoved(int x, int y, int deltaX, int deltaY, 
		bool leftBtnDown, bool rightBtnDown, int fwKeys)
	{
		Vector2 vpos(x,y);
		pContext->pViewport->unproject(&vpos);
		vecObjectsUnderMouse.clear();
		pContext->pBSPTree->retrieveWorldObjectsInBox(AlignedBox(vpos, vpos), &vecObjectsUnderMouse);
		pCurrentObj = NULL;
		unsigned i=0; while (i<vecObjectsUnderMouse.size() && !vecObjectsUnderMouse[i]->isSelectable()) ++i;
		if (i<vecObjectsUnderMouse.size())
			pCurrentObj = vecObjectsUnderMouse[i];
		if (pCurrentObj == NULL) {
			if (selectionRectVisible[which_end-1]) {
				pContext->pViewport->removePermanentOSDElement(pSelectionRect[which_end-1]);
				selectionRectVisible[which_end-1] = false;
			}
		} else {
			updateSelectionRect();
		}
		return false;
	}

	bool ResortOperation::ResortAttachEnd::mouseWheel(int x, int y, float zDelta, int fwKeys)
	{
		/*if (selectionRectVisible[which_end-1]) {
			updateSelectionRect();
		}*/
		return false;
	}

	bool ResortOperation::ResortAttachEnd::mouseDown(MouseButton button, int x, int y, int fwKeys)
	{
		clicked = (button == MOUSE_LEFTBTN && fwKeys==MK_LBUTTON);
		return clicked;
	}

	bool ResortOperation::ResortAttachEnd::mouseUp(MouseButton button, int x, int y, int fwKeys) 
	{
		bool was_clicked = clicked;
		clicked = false;
		if (was_clicked && button == MOUSE_LEFTBTN && selectionRectVisible[which_end-1]) {
			// attach the n-th end of the resort here
			switch (which_end) {
			case 1: {
				Vector2 vPos(x,y);
				pContext->pViewport->unproject(&vPos);
				pContext->pStack->swapTopOperation(new ResortSetLength(vPos, pCurrentObj));
				} break;
			case 2:
				break;
			default:
				assert(false); // this should *NEVER* happen
			}
			return true;
		} else
			return false;
	}
	void ResortOperation::ResortAttachEnd::leave() 
	{
		for (int i=0; i<2; ++i) {
			pContext->pRenderer->unregisterRenderable(pSelectionRect[i]);
			delete pSelectionRect[i];
			if (which_end == 1)
				break;
		}
	}
	void ResortOperation::ResortAttachEnd::activate()
	{
		msgID = pContext->pRenderer->getOSDTextManager()->addLine(
			which_end == 1 
				? L"Click on an object to attach the 1st end of the resort"
				: L"Click on an object to attach the 2nd end of the resort"
				);
		updateSelectionRect();
	}
	void ResortOperation::ResortAttachEnd::deactivate()
	{
		for (int i=0; i<2; ++i) {
			if (selectionRectVisible[i]) {
				pContext->pViewport->removePermanentOSDElement(pSelectionRect[i]);
				selectionRectVisible[i] = false;
			}
			if (which_end == 1)
				break;
		}
		pContext->pRenderer->getOSDTextManager()->removeLine(msgID);
	}

	void ResortOperation::ResortAttachEnd::updateSelectionRect()
	{
		if (pCurrentObj != NULL) {
			AlignedBox bbox; pCurrentObj->getBoundingBox(&bbox);
			pContext->pViewport->project(&bbox.bottomLeft);
			pContext->pViewport->project(&bbox.topRight);
			pSelectionRect[which_end-1]->fromBox(bbox);
		}
		for (int i=0; i<2; ++i) {
			if (!selectionRectVisible[i]) {
				pContext->pViewport->addPermanentOSDElement(pSelectionRect[i],
					Render::ANCHOR_LEFTTOP, 0, 0);
				selectionRectVisible[i] = true;
			}
			if (which_end == 1)
				break;
		}
	}

/// set length sub-operation ----------------------------------------------

	ResortOperation::ResortSetLength::ResortSetLength(Vector2 &vFirstEnd, WorldObject* pFirstObj)
		: pContext(NULL)
		, msgID(-1)
		, pFirstObject(pFirstObj)
		, vFirstEnd(vFirstEnd)
		, pSelectionRect(NULL)
		, pResortLine(NULL)
		, clicked(false)
	{
	}

	void ResortOperation::ResortSetLength::enter(const OperationContext* pContext)
	{
		this->pContext = pContext;
		pSelectionRect = new Render::Rectangle(Vector2());
		pContext->pRenderer->registerRenderable(pSelectionRect);
		pSelectionRect->lineColor = 0xFF00FF00;
		AlignedBox bbox; pFirstObject->getBoundingBox(&bbox);
		pContext->pViewport->project(&bbox.bottomLeft);
		pContext->pViewport->project(&bbox.topRight);
		pSelectionRect->fromBox(bbox);
		pResortLine = new Render::Line(Vector2(), Vector2());
		pContext->pRenderer->registerRenderable(pResortLine);
		pResortLine->lineColor = 0xFF00FF00;

		pContext->pViewport->addPermanentOSDElement(pSelectionRect, Render::ANCHOR_LEFTTOP, 0, 0);
		pContext->pViewport->addPermanentOSDElement(pResortLine, Render::ANCHOR_LEFTTOP, 0, 0);
	}

	void ResortOperation::ResortSetLength::leave()
	{
		pContext->pViewport->removePermanentOSDElement(pSelectionRect);
		pContext->pRenderer->unregisterRenderable(pSelectionRect);
		pContext->pViewport->removePermanentOSDElement(pResortLine);
		pContext->pRenderer->unregisterRenderable(pResortLine);

		delete pSelectionRect;
		delete pResortLine;
	}

	void ResortOperation::ResortSetLength::activate()
	{
		msgID = pContext->pRenderer->getOSDTextManager()->addLine(
			L"Click anywhere to set the initial length of the resort");
	}

	void ResortOperation::ResortSetLength::deactivate()
	{
		pContext->pRenderer->getOSDTextManager()->removeLine(msgID);
	}

	bool ResortOperation::ResortSetLength::mouseMoved(int x, int y, int deltaX, int deltaY, 
		bool leftBtnDown, bool rightBtnDown, int fwKeys)
	{
		Vector2 v1(vFirstEnd), v2(x,y);
		pContext->pViewport->project(&v1);
		pResortLine->setPoints(v1, v2);
		return false;
	}

	bool ResortOperation::ResortSetLength::mouseDown(MouseButton button, int x, int y, int fwKeys)
	{
		clicked = (button == MOUSE_LEFTBTN && fwKeys==MK_LBUTTON);
		return clicked;
	}

	bool ResortOperation::ResortSetLength::mouseUp(MouseButton button, int x, int y, int fwKeys)
	{
		bool was_clicked = clicked;
		clicked = false;
		if (clicked && button == MOUSE_LEFTBTN) {
			Vector2 v2(x,y);
			pContext->pViewport->unproject(&v2);
			double length = glm::length(v2-vFirstEnd);
			pContext->pStack->swapTopOperation(new ResortAttachEnd(2, pFirstObject, vFirstEnd, length));
			return true;
		} else
			return false;
	}

/// main operation -----------------------------------------------------------------

	ResortOperation::ResortOperation(HINSTANCE hInstance)
		: isConsumed(false)
		, hInstance(hInstance)
	{
		m_popupMenuRoot = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_RESORT_OP_POPUP));
		if (m_popupMenuRoot == NULL)
			throw wstring(L"could not load popup menu!");
		m_popupMenu = GetSubMenu(m_popupMenuRoot, 0);
	}
	
	ResortOperation::~ResortOperation()
	{
		DestroyMenu(m_popupMenuRoot);
	}

	void ResortOperation::enter(const OperationContext* pContext)
	{
		this->pContext = pContext;
		pContext->pStack->pushOperation(new ResortAttachEnd(1, NULL, Vector2(), 0));
	}

	void ResortOperation::leave()
	{
	}

	void ResortOperation::activate()
	{
		if (isConsumed)
			pContext->pStack->removeTopOperation();
	}

	void ResortOperation::deactivate()
	{
		isConsumed = true; // sub-operation is in effect now
	}

	bool ResortOperation::mouseDown(MouseButton button, int x, int y, int fwKeys)
	{
		if (button == MOUSE_RIGHTBTN) {
			return true;
		} else
			return false;
	}

	bool ResortOperation::mouseUp(MouseButton button, int x, int y, int fwKeys)
	{
		if (button == MOUSE_RIGHTBTN) {
			POINT xyPoint; xyPoint.x = x; xyPoint.y = y;
			ClientToScreen(pContext->renderTarget, &xyPoint);
			TrackPopupMenu(m_popupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, xyPoint.x, xyPoint.y, 0, 
				pContext->renderTarget, NULL);
			return true;
		} else
			return false;
	}

	bool ResortOperation::command(int cmd, LPARAM lParam)
	{
		switch (cmd) {
			case ID_RESORT_OP_CANCEL: {
				isConsumed = false; // prevent deletion on activation
				while (pContext->pStack->getTopOperation() != this)
					pContext->pStack->removeTopOperation();
				pContext->pStack->removeTopOperation();
				}; break;
			default:
				return false;
		};
		return true;
	}

}
