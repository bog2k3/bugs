#include "stdafx.h"
#include "Line.h"
#include "../IRenderer.h"

namespace Render
{
	int Line::resourceRefCount = 0;
	ID3DXLine* Line::pLine = NULL;

	Line::Line(Vector2 &p1, Vector2 &p2)
		: m_anchorX(100)
		, m_anchorY(100)
		, m_Anchor(ANCHOR_LEFTTOP)
		, lineColor(0xFFA0A0A0)
	{
		setPoints(p1, p2);
	}

	Line::~Line()
	{
	}

	void Line::setPoints(Vector2 &p1, Vector2 &p2)
	{
		this->p1 = p1;
		this->p2 = p2;
		m_rect.left = (int)min(p1.x, p2.x);
		m_rect.top = (int)min(p1.y, p2.y);
		m_rect.right = (int)max(p1.x, p2.x);
		m_rect.bottom = (int)max(p1.y, p2.y);
	}

	void Line::initializeResources(IDirect3DDevice9* pDevice)
	{
		recreateHWResourcePostReset(pDevice);
	}

	void Line::releaseResources()
	{
		releaseHWResourcesPreReset();
	}

	void Line::releaseHWResourcesPreReset()
	{
		if (--resourceRefCount == 0) {
			pLine->Release();
			pLine = NULL;
		}
	}

	void Line::recreateHWResourcePostReset(IDirect3DDevice9* pDevice)
	{
		if (resourceRefCount++ == 0) {
			D3DXCreateLine(pDevice, &pLine);
			pLine->SetAntialias(FALSE);
		}
	}

	bool Line::isInsideViewport(int vWidth, int vHeight)
	{
		return m_rect.left < vWidth && m_rect.top < vHeight
			&& m_rect.right >= 0 && m_rect.bottom >= 0;
	}

	void Line::renderOSD(RenderContext* pRenderContext)
	{
		assert (pLine != NULL);

		pLine->Begin();
		D3DXVECTOR2 verts[2] = {
			D3DXVECTOR2((float)p1.x, (float)p1.y),
			D3DXVECTOR2((float)p2.x, (float)p2.y),
		};
		pLine->Draw(verts, 2, lineColor);
		pLine->End();
	}

	void Line::setPosition(int x, int y, LayoutAnchorEnum anchor)
	{
		m_anchorX = x;
		m_anchorY = y;
		m_Anchor = anchor;
	}

}