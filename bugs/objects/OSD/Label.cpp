#include "Label.h"

Label::Label(wstring text, D3DCOLOR color)
	: m_text(text)
	, m_textColor(color)
	, m_textX(0)
	, m_textY(0)
	, m_anchorX(0)
	, m_anchorY(0)
{
}

Label::~Label()
{
}

void Label::preRender(RenderContext* pCtxt)
{
}

void Label::render(RenderContext* pCtxt)
{
}

void Label::postRender(RenderContext* pCtxt)
{
}

void Label::initializeResources(IDirect3DDevice9* pDevice)
{
}

void Label::releaseResources()
{
}

void Label::releaseHWResourcesPreReset()
{
}

void Label::recreateHWResourcePostReset(IDirect3DDevice9* pDevice)
{
}

bool Label::isInsideViewport(int vWidth, int vHeight)
{
	return m_textX >= 0 && m_textX < vWidth
		&& m_textY >= 0 && m_textY < vHeight;
}

void Label::renderOSD(RenderContext* pRenderContext)
{
	pRenderContext->getFont()->DrawTextW(pRenderContext->getSprite(), m_text.c_str(), -1,
		&m_rect, DT_LEFT|DT_TOP|DT_NOCLIP, m_textColor);
}

void Label::setPosition(int x, int y, LayoutAnchorEnum anchor)
{
	m_anchorX = x; m_anchorY = y;
	m_textX = m_anchorX; m_textY = m_anchorY;
	SetRect(&m_rect, m_textX, m_textY, 0, 0);
}

void Label::getRect(RECT* rc)
{
	//TODO implement
}

void Label::setText( wstring text )
{
	m_text = text;
}
