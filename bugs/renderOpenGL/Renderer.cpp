/*
 * Renderer.cpp
 *
 *  Created on: Nov 2, 2014
 *      Author: bog
 */

#include "Renderer.h"
#include "Viewport.h"
#include <stdexcept>

Renderer::~Renderer() {
	// TODO Auto-generated destructor stub
}

Renderer::Renderer() {
}

void Renderer::registerRenderable(IRenderable* r) {
	if (r == nullptr)
		throw new std::invalid_argument("argument cannot be null!");
	renderComponents.push_back(r);
}

void Renderer::render() {
	for (auto vp : viewports) {
		if (!vp->isEnabled())
			continue;
		for (auto r : renderComponents) {
			r->render(vp);
		}
	}
	for (auto r : renderComponents) {
		r->purgeRenderQueue();
	}
}
