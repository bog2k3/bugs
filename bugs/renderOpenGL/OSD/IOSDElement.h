#pragma once

#include "../IRenderable.h"

struct D3DXVECTOR2;

namespace Render
{
	class IViewport;
	enum LayoutAnchorEnum;

	class IOSDElement abstract : public virtual IRenderable
	{
	public:
		virtual ~IOSDElement() {}

		// return true if the element is at least partially visible inside the viewport
		// return false if the element is completely outside the viewport
		virtual bool isInsideViewport(int vWidth, int vHeight)=0;

		// do the OSD related rendering here; rendering is done in viewport screen space
		// use the sprite passed in the render context for text & bitmap drawing
		virtual void renderOSD(RenderContext* pRenderContext)=0;

		// set the position in viewport screen space
		virtual void setPosition(int x, int y, LayoutAnchorEnum anchor)=0;

		// returns the current anchor of the element
		virtual LayoutAnchorEnum getAnchor()=0;
		// return the position of the anchor, as set previuously by setPosition
		virtual void getAnchorPosition(D3DXVECTOR2* vPos)=0;
		// return the bounding rectangle of the element, in viewport space
		virtual void getRect(RECT* rc)=0;

		// called once at the begining; create all resources here
		virtual void initializeResources(IDirect3DDevice9* pDevice)=0;
		// called once before destruction; release all resources here
		virtual void releaseResources()=0;

		// called before a device reset; release POOL_DEFAULT resources here
		virtual void releaseHWResourcesPreReset()=0;
		// called after a device reset; recreate POOL_DEFAULT resources here
		virtual void recreateHWResourcePostReset(IDirect3DDevice9* pDevice)=0;

	protected:
		virtual void preRender(RenderContext* pCtxt) {}
		virtual void render(RenderContext* pCtxt) {}
		virtual void postRender(RenderContext* pCtxt) {}
	};
}