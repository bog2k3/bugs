#include "Viewport.h"
#include "Camera.h"

#include <glm/vec2.hpp>

using namespace glm;

Viewport::Viewport(int x, int y, int w, int h)
	: m_userData(0)
	, viewportArea(x, y, w, h)
	, pCamera(new Camera(this))
	, mEnabled(true)
{
}

Viewport::~Viewport()
{
	delete pCamera;
}

void Viewport::setArea(int vpX, int vpY, int vpW, int vpH)
{
	viewportArea = vec4(vpX, vpY, vpW, vpH);

	pCamera->updateProj();
}

vec3 Viewport::unproject(vec3 point) const
{
	assert(false); // TODO restore

	/*vec2 vCamPos = pCamera->getPos();
	point.x = vCamPos.x + _1mag*(point.x - viewportArea.z*0.5f);
	point.y = vCamPos.y + _1mag*(viewportArea.w*0.5 - point.y);
	return point;*/
}

vec3 Viewport::project(vec3 point) const
{
	assert(false); // TODO restore

	/*vec2 vCamPos = pCamera->getPos();
	point -= vCamPos;
	point *= pCamera->getZoomLevel();
	point.x += viewportArea.z * 0.5f;
	point.y = viewportArea.w * 0.5f - point.y;
	return point;*/
}

bool Viewport::containsPoint(glm::vec2 const&p) const {
	return p.x >= viewportArea.x && p.y >= viewportArea.y &&
			p.x <= viewportArea.x + viewportArea.z &&
			p.y <= viewportArea.y + viewportArea.w;
}
