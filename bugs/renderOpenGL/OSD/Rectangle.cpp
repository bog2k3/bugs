#include "stdafx.h"
#include "Rectangle.h"
#include "../IRenderer.h"

namespace Render
{
	int Rectangle::resourceRefCount = 0;
	ID3DXLine* Rectangle::pLine = NULL;

	Rectangle::Rectangle(Vector2 &size)
		: m_anchorX(100)
		, m_anchorY(100)
		, m_Anchor(ANCHOR_LEFTTOP)
		, m_size(size)
		, textColor(0xFF2080FF)
		, lineColor(0xFFA0A0A0)
		, labelYOffset(0)
	{
	}

	Rectangle::~Rectangle()
	{
	}

	// sets the rectangle's corners like the box
	void Rectangle::fromBox(AlignedBox &box)
	{
		SetRect(&m_rect, (int)box.bottomLeft.x, (int)box.topRight.y, (int)box.topRight.x, (int)box.bottomLeft.y);
		m_size.x = m_rect.right - m_rect.left;
		m_size.y = m_rect.bottom - m_rect.top;
	}

	void Rectangle::initializeResources(IDirect3DDevice9* pDevice)
	{
		recreateHWResourcePostReset(pDevice);
	}

	void Rectangle::releaseResources()
	{
		releaseHWResourcesPreReset();
	}

	void Rectangle::releaseHWResourcesPreReset()
	{
		if (--resourceRefCount == 0) {
			pLine->Release();
			pLine = NULL;
		}
	}

	void Rectangle::recreateHWResourcePostReset(IDirect3DDevice9* pDevice)
	{
		if (resourceRefCount++ == 0) {
			D3DXCreateLine(pDevice, &pLine);
			pLine->SetAntialias(FALSE);
		}
	}

	bool Rectangle::isInsideViewport(int vWidth, int vHeight)
	{
		return m_rect.left < vWidth && m_rect.top < vHeight
			&& m_rect.right >= 0 && m_rect.bottom >= 0;
	}

	void Rectangle::renderOSD(RenderContext* pRenderContext)
	{
		assert (pLine != NULL);

		pLine->Begin();
		D3DXVECTOR2 verts[5] = {
			D3DXVECTOR2((float)m_rect.left, (float)m_rect.bottom),
			D3DXVECTOR2((float)m_rect.left, (float)m_rect.top),
			D3DXVECTOR2((float)m_rect.right, (float)m_rect.top),
			D3DXVECTOR2((float)m_rect.right, (float)m_rect.bottom),
			D3DXVECTOR2((float)m_rect.left, (float)m_rect.bottom)
		};
		pLine->Draw(verts, 5, lineColor);
		pLine->End();
	}

	void Rectangle::setPosition(int x, int y, LayoutAnchorEnum anchor)
	{
		m_anchorX = x;
		m_anchorY = y;
		m_Anchor = anchor;
	}

}