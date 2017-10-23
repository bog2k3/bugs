#include "Viewport.h"
#include "Camera.h"

using namespace glm;

Viewport::Viewport(int x, int y, int w, int h)
	: userData_(0)
	, viewportArea_(x, y, w, h)
	, pCamera_(new Camera(this))
	, enabled_(true)
{
}

Viewport::~Viewport()
{
	delete pCamera_;
}

void Viewport::setArea(int vpX, int vpY, int vpW, int vpH)
{
	viewportArea_ = vec4(vpX, vpY, vpW, vpH);

	pCamera_->updateProj();
}

vec3 Viewport::unproject(vec3 point) const
{
	vec4 unif {point, 1};
	unif.x /= viewportArea_.z * 0.5f;
	unif.y /= viewportArea_.w * 0.5f;

	auto camPV = camera()->matProjView();
	if (mPV_cache_ != camPV) {
		mPV_cache_ = camPV;
		mPV_inv_cache_ = glm::inverse(camPV);
	}

	auto ret = mPV_inv_cache_ * unif;
	return {ret.x, ret.y, ret.z};
}

vec3 Viewport::project(vec3 point) const
{
	auto matPV = camera()->matProjView();
	auto unif = matPV * vec4{point, 1};
	vec3 ret { unif.x, unif.y, unif.z };
	ret *= 1.f / unif.w;
	ret.x *= viewportArea_.z * 0.5f;
	ret.y *= viewportArea_.w * 0.5f;
	return ret;
}

bool Viewport::containsPoint(glm::vec2 const&p) const {
	return p.x >= viewportArea_.x && p.y >= viewportArea_.y &&
			p.x <= viewportArea_.x + viewportArea_.z &&
			p.y <= viewportArea_.y + viewportArea_.w;
}
