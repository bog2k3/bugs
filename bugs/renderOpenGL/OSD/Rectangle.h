#pragma once

#include "../OSD/IOSDElement.h"
#include <d3dx9.h>
#include "../../math/math.h"
#include "../IViewport.h"

struct ID3DXLine;

namespace Render
{
	class Rectangle : public IOSDElement
	{
	public:
		Rectangle(Vector2 &size);
		virtual ~Rectangle();

		// sets the rectangle's corners like the box
		void fromBox(AlignedBox &box);

		D3DCOLOR textColor;
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

		//TODO  use a label bottom-right; make this label user-editable;
		// also make this label represent the selected object's name; have all objects named
		int labelYOffset;

		Vector2 m_size;
		RECT m_rect;

		static ID3DXLine* pLine;
		static int resourceRefCount; // reference counter for shared video resources
	};
}
