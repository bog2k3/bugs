#pragma once

#include "IOSDElement.h"
#include <string>

class Label : public IOSDElement
{
public:
	Label(std::string text, D3DCOLOR color);
	virtual ~Label();

	virtual void preRender(RenderContext* pCtxt);
	virtual void render(RenderContext* pCtxt);
	virtual void postRender(RenderContext* pCtxt);

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

	void setText(wstring text);

protected:
	wstring m_text;
	RECT m_rect;
	int m_textX, m_textY;
	int m_anchorX, m_anchorY;
	LayoutAnchorEnum m_Anchor;
	D3DCOLOR m_textColor;
};
