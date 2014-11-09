#include "stdafx.h"

#include "ScaleDisplay.h"
#include "../RenderContext.h"
#include "../IViewport.h"
#include "../ICamera.h"
#include "../IRenderer.h"
#include <d3dx9.h>

namespace Render
{
	ScaleDisplay::ScaleDisplay(int maxPixelSize)
		: m_textColor(0xFFFFFFFF)
		, m_lineColor(0xFFA0A0A0)
		, m_MaxSize(maxPixelSize)
		, pLine(NULL)
		, segmentsXOffset(50)
		, segmentHeight(10)
		, labelYOffset(-12)
		, m_drawX(100), m_anchorX(100)
		, m_drawY(100), m_anchorY(100)
		, m_Anchor(ANCHOR_LEFTBOTTOM)
	{
	}

	ScaleDisplay::~ScaleDisplay()
	{
	}

	void ScaleDisplay::initializeResources(IDirect3DDevice9* pDevice)
	{
		recreateHWResourcePostReset(pDevice);
	}

	void ScaleDisplay::releaseResources()
	{
		releaseHWResourcesPreReset();
	}

	void ScaleDisplay::releaseHWResourcesPreReset()
	{
		pLine->Release();
		pLine = NULL;
	}

	void ScaleDisplay::recreateHWResourcePostReset(IDirect3DDevice9* pDevice)
	{
		D3DXCreateLine(pDevice, &pLine);
		pLine->SetAntialias(FALSE);
	}

	bool ScaleDisplay::isInsideViewport(int vWidth, int vHeight)
	{
		return true; //TODO fix
	}

	void ScaleDisplay::getRect(RECT* rc)
	{
		//TODO implement
	}

	void ScaleDisplay::renderOSD(RenderContext* pRenderContext)
	{
		double pixelsPerUnit = pRenderContext->getViewport()->getCamera()->getMagnification();
		int exponent = 0;
		
		if (pixelsPerUnit > m_MaxSize) {
			// small scale
			while (pixelsPerUnit > m_MaxSize) {
				exponent--;
				pixelsPerUnit /= 10;
			}
		} else if (pixelsPerUnit < m_MaxSize) {
			// large scale
			while (pixelsPerUnit*10 <= m_MaxSize) {
				exponent++;
				pixelsPerUnit *= 10;
			}
		}

		float segIncrement = 1.0f;
		int segments = (int) floor(m_MaxSize / pixelsPerUnit);
		if (segments == 1) {
			segments = 5;
			segIncrement = 0.2f;
		/*} else if (segments == 2) {
			segments = 8;
			segIncrement = 0.25f;*/
		} else if (segments <= 3) {
			segments *= 2;
			segIncrement = 0.5f;
		}
		int nVertex = 1 + segments * 3;
		float cx = (float)m_drawX + segmentsXOffset;
		float cy = (float)m_drawY - 1;
		D3DXVECTOR2 vList[31]; // 31 is max vertex for max_seg=10
		for (int i=0; i<segments; i++) {
			int localSegHeight = (int)(i*segIncrement) == (i*segIncrement) ? segmentHeight : segmentHeight / 2;
			vList[i*3+0] = D3DXVECTOR2(cx, cy-localSegHeight);
			vList[i*3+1] = D3DXVECTOR2(cx, cy);
			cx += (float)pixelsPerUnit * segIncrement;
			vList[i*3+2] = D3DXVECTOR2(cx, cy);
		}
		vList[nVertex-1] = D3DXVECTOR2(cx, cy-segmentHeight);

		pLine->Begin();
		pLine->Draw(vList, nVertex, m_lineColor);
		pLine->End();

		wchar_t scaleLabel[100];

		swprintf_s(scaleLabel, 100, L"(10^%d)", exponent);
		SetRect(&m_rect, m_drawX, m_drawY+labelYOffset, m_drawX, m_drawY+labelYOffset);
		pRenderContext->getFont()->DrawTextW(pRenderContext->getSprite(),
			scaleLabel, -1, &m_rect, DT_BOTTOM|DT_NOCLIP, m_textColor);
		for (int i=0; i<segments+1; i++) {
			swprintf_s(scaleLabel, 100, L"%g", i*segIncrement);
			int localSegHeight = (int)(i*segIncrement) == (i*segIncrement) ? 0 : segmentHeight / 2;
			SetRect(&m_rect, m_drawX+segmentsXOffset+i*(int)(pixelsPerUnit*segIncrement), 
				m_drawY+labelYOffset + localSegHeight, 
				m_drawX+segmentsXOffset+i*(int)(pixelsPerUnit*segIncrement), 
				m_drawY+labelYOffset + localSegHeight);
			pRenderContext->getFont()->DrawTextW(pRenderContext->getSprite(),
				scaleLabel, -1, &m_rect, DT_BOTTOM|DT_NOCLIP, m_textColor);
		}
	}

	void ScaleDisplay::setPosition(int x, int y, LayoutAnchorEnum anchor)
	{
		m_anchorX = x;
		m_anchorY = y;
		m_Anchor = anchor;

		if (anchor & ANCHOR_LEFT)
			m_drawX = x;
		else
			if (anchor & ANCHOR_RIGHT)
				m_drawX = x - m_MaxSize;
			else
				assert(FALSE && "invalid anchor!");
		if (anchor & ANCHOR_TOP)
			m_drawY = y + labelYOffset + 35;
		else
			if (anchor & ANCHOR_BOTTOM)
				m_drawY = y;
			else
				assert(FALSE && "invalid anchor!");
	}

}