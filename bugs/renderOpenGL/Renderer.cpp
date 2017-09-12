/*
 * Renderer.cpp
 *
 *  Created on: Nov 2, 2014
 *      Author: bog
 */

#include "Renderer.h"
#include "Viewport.h"
#include "Shape2D.h"
#include "Shape3D.h"
#include "GLText.h"
#include "MeshRenderer.h"
#include "../utils/DrawList.h"
#include "../utils/assert.h"

#include "GL/gl.h"

#include <stdexcept>

Renderer::~Renderer() {
}

Renderer::Renderer(int winW, int winH)
	: winW_(winW), winH_(winH)
{
	Shape2D::init(this);
	Shape3D::init(this);
	GLText::init(this, "data/fonts/DejaVuSansMono_256_16_8.png", 8, 16, ' ', 22);
	MeshRenderer::init(this);
}

void Renderer::registerRenderable(IRenderable* r) {
	if (r == nullptr)
		throw new std::invalid_argument("argument cannot be null!");
	renderComponents_.push_back(r);
}

void Renderer::addViewport(std::string name, std::unique_ptr<Viewport> vp) {
	vp->setName(name);
	viewports_[name] = std::move(vp);
}

Viewport* Renderer::getViewport(std::string name) const {
	auto it = viewports_.find(name);
	if (it == viewports_.end()) {
		return nullptr;
	} else
		return it->second.get();
}

std::vector<Viewport*> Renderer::getViewports() const {
	std::vector<Viewport*> ret;
	for (auto &p : viewports_)
		ret.push_back(p.second.get());
	return ret;
}

void Renderer::deleteViewport(std::string const& name) {
	auto it = viewports_.find(name);
	assertDbg(it != viewports_.end() && "non existing viewport");
	viewports_.erase(it);
}

void Renderer::render() {
	// first clear viewports:
	for (auto &vp : viewports_) {
		if (!vp.second->isEnabled())
			continue;
		auto vpp = vp.second->position();
		glViewport(vpp.x, vpp.y, vp.second->width(), vp.second->height());
		glClearColor(vp.second->backgroundColor_.r, vp.second->backgroundColor_.g, vp.second->backgroundColor_.b, 0);
		glScissor(vpp.x, vpp.y, vp.second->width(), vp.second->height());
		glEnable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
	}
	for (auto r : renderComponents_) {
		for (auto &vp : viewports_) {
			if (!vp.second->isEnabled())
				continue;
			auto vpp = vp.second->position();
			glViewport(vpp.x, vpp.y, vp.second->width(), vp.second->height());
			r->render(vp.second.get());
		}
		r->purgeRenderQueue();
	}
}

void Renderer::unload() {
	for (auto r : renderComponents_) {
		r->purgeRenderQueue();
		r->unload();
	}
}
