#include "Viewport.h"
#include "Camera.h"

using namespace glm;

Viewport::Viewport(int x, int y, int w, int h)
	: m_userData(0)
	, viewportArea(x, y, w, h)
	, pCamera(new Camera(this))
	, fScale(1e-2)
	, mEnabled(true)
{
}

Viewport::~Viewport()
{
	delete pCamera;
}

Camera* Viewport::getCamera()
{
	return pCamera;
}

void Viewport::setScale(double scale)
{
	fScale = scale;

	// scale is relative to the minimum edge of the viewport:
	double size = (double)(viewportArea.z > viewportArea.w ? viewportArea.z : viewportArea.w);

	pCamera->setZoomLevel(size / (scale * 10.0));
}

void Viewport::setArea(int vpX, int vpY, int vpW, int vpH)
{
	// int oldW = viewportArea.z;
	// int oldH = viewportArea.w;
	viewportArea = vec4(vpX, vpY, vpW, vpH);

	// refreshOSDLayout(oldW, oldH);
	setScale(fScale);
}

/*void Viewport::refreshOSDLayout(int oldWidth, int oldHeight)
{
	int dW = d3dViewport.Width - oldWidth;
	int dH = d3dViewport.Height - oldHeight;
	std::vector<IOSDElement*>::iterator
		it=OSDPerm_vec.begin(),
		itEnd=OSDPerm_vec.end();
	for (; it != itEnd; ++it) {
		LayoutAnchorEnum anchor = (*it)->getAnchor();
		D3DXVECTOR2 vPos; (*it)->getAnchorPosition(&vPos);
		if (anchor & ANCHOR_RIGHT)
			vPos.x += dW;
		if (anchor & ANCHOR_BOTTOM)
			vPos.y += dH;
		(*it)->setPosition((int)vPos.x, (int)vPos.y, anchor);
	}
}*/

void Viewport::pan(vec2 delta)
{
	double mag = pCamera->getZoomLevel();
	pCamera->move(vec2(-delta.x / mag, delta.y / mag));
}

vec2 Viewport::unproject(vec2 point)
{
	double _1mag = 1.0 / pCamera->getZoomLevel();
	vec2 vCamPos = pCamera->getPos();
	point.x = vCamPos.x + _1mag*(point.x - viewportArea.z*0.5f);
	point.y = vCamPos.y + _1mag*(viewportArea.w*0.5 - point.y);
	return point;
}

vec2 Viewport::project(vec2 point)
{
	vec2 vCamPos = pCamera->getPos();
	point -= vCamPos;
	point *= pCamera->getZoomLevel();
	point.x += viewportArea.z * 0.5f;
	point.y = viewportArea.w * 0.5f - point.y;
	return point;
}

/*void Viewport::queueOSDElement(IOSDElement* pElement, LayoutAnchorEnum anchor, int Xpos, int Ypos)
{
	int elemX, elemY;
	if (anchor & ANCHOR_LEFT)
		elemX = Xpos;
	else
		if (anchor & ANCHOR_RIGHT)
			elemX = d3dViewport.Width - Xpos;
		else
			assert(FALSE && "invalid anchor!");
	if (anchor & ANCHOR_TOP)
		elemY = Ypos;
	else
		if (anchor & ANCHOR_BOTTOM)
			elemY = d3dViewport.Height - Ypos;
		else
			assert(FALSE && "invalid anchor!");
	pElement->setPosition(elemX, elemY, anchor);

	EnterCriticalSection(&cs_OSDElements);
	OSD_vec.push_back(pElement);
	LeaveCriticalSection(&cs_OSDElements);
}

void Viewport::addPermanentOSDElement(IOSDElement* pElement, LayoutAnchorEnum anchor,
	int Xpos, int Ypos)
{
	int elemX, elemY;
	if (anchor & ANCHOR_LEFT)
		elemX = Xpos;
	else
		if (anchor & ANCHOR_RIGHT)
			elemX = d3dViewport.Width - Xpos;
		else
			assert(FALSE && "invalid anchor!");
	if (anchor & ANCHOR_TOP)
		elemY = Ypos;
	else
		if (anchor & ANCHOR_BOTTOM)
			elemY = d3dViewport.Height - Ypos;
		else
			assert(FALSE && "invalid anchor!");
	pElement->setPosition(elemX, elemY, anchor);

	EnterCriticalSection(&cs_OSDElements);
	OSDPerm_vec.push_back(pElement);
	LeaveCriticalSection(&cs_OSDElements);
}

void Viewport::removePermanentOSDElement(IOSDElement* pElement)
{
	std::vector<IOSDElement*>::iterator
		it=OSDPerm_vec.begin(),
		itEnd=OSDPerm_vec.end();
	for (; it != itEnd; ++it)
		if (*it == pElement) {
			EnterCriticalSection(&cs_OSDElements);
			OSDPerm_vec.erase(it);
			LeaveCriticalSection(&cs_OSDElements);
			break;
		}
}

void Viewport::renderOSD(RenderContext* pRenderContext)
{
	EnterCriticalSection(&cs_OSDElements);
	std::vector<IOSDElement*>::iterator
		it=OSDPerm_vec.begin(),
		itEnd=OSDPerm_vec.end();
	int vpW = d3dViewport.Width;
	int vpH = d3dViewport.Height;
	for (; it != itEnd; ++it) {
		if ((*it)->isInsideViewport(vpW, vpH))
			(*it)->renderOSD(pRenderContext);
	}
	it=OSD_vec.begin(),
	itEnd=OSD_vec.end();
	for (; it != itEnd; ++it) {
		if ((*it)->isInsideViewport(vpW, vpH))
			(*it)->renderOSD(pRenderContext);
	}
	OSD_vec.clear();
	LeaveCriticalSection(&cs_OSDElements);
}
*/
