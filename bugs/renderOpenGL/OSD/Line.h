#pragma once

#include "../OSD/IOSDElement.h"
#include <d3dx9.h>
#include "../../math/math.h"
#include "../IViewport.h"

struct ID3DXLine;

namespace Render
{
	class Line : public IOSDElement
	{
	public:
		Line(Vector2 &p1, Vector2 &p2);
		virtual ~Line();

		void setPoints(Vector2 &p1, Vector2 &p2);

		D3DCOLOR lineColor;

		virtual void initializeResources(IDirect3DDevice9* pDevice);
		virtual void releaseResources();

		virtual void releaseHWResourcesPreReset();
		virtual void recreateHWResourcePostReset(IDirect3DDevice9* pDevice);

		virtual bool isInsideViewport(int vWidth, int vHeight);
		virtual void renderOSD(RenderContext* pRenderContext);
		virtual void setPosition(int x, int y, LayoutAnchorEnum anchor);
		virtual LayoutAnchorEnum getAnchor() { return m_Anchor; }
		virtual void getAnchorPosition(D3DXVECTOR2* vPos) { vPos->x = (float)m_anchorX; vPos->y = (float)m_anchorY; }
		virtual void getRect(RECT* rc) { *rc = m_rect; }

	protected:
		int m_anchorX, m_anchorY;
		LayoutAnchorEnum m_Anchor;

		Vector2 p1, p2;
		RECT m_rect;

		static ID3DXLine* pLine;
		static int resourceRefCount; // reference counter for shared video resources
	};
}