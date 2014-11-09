#pragma once

#include "../OSD/IOSDElement.h"
#include <d3dx9.h>

struct ID3DXLine;

namespace Render
{
	class ScaleDisplay : public IOSDElement
	{
	public:
		ScaleDisplay(int maxPixelSize);
		virtual ~ScaleDisplay();

		virtual void initializeResources(IDirect3DDevice9* pDevice);
		virtual void releaseResources();
		virtual void releaseHWResourcesPreReset();
		virtual void recreateHWResourcePostReset(IDirect3DDevice9* pDevice);

		virtual bool isInsideViewport(int vWidth, int vHeight);
		virtual void renderOSD(RenderContext* pRenderContext);
		virtual void setPosition(int x, int y, LayoutAnchorEnum anchor);
		virtual LayoutAnchorEnum getAnchor() { return m_Anchor; }
		virtual void getAnchorPosition(D3DXVECTOR2* vPos) { vPos->x = (float)m_anchorX; vPos->y = (float)m_anchorY; }
		virtual void getRect(RECT* rc);

	protected:
		// TODO: make multi-instance safe (share video resources)
		int m_drawX, m_drawY;
		int m_anchorX, m_anchorY;
		LayoutAnchorEnum m_Anchor;
		int segmentsXOffset;
		int segmentHeight;
		int labelYOffset;
		ID3DXLine* pLine;
		RECT m_rect;
		D3DCOLOR m_textColor;
		D3DCOLOR m_lineColor;
		int m_MaxSize;
	};
}