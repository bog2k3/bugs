/*
 * Renderer.h
 *
 *  Created on: Nov 2, 2014
 *      Author: bog
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#include "IRenderable.h"
#include <vector>
#include <functional>


class Renderer {
public:
	virtual ~Renderer();
	Renderer();

	void registerRenderable(IRenderable* r);
	void addViewport(Viewport* vp);
	void render();

protected:
	std::vector<IRenderable*> renderComponents;
	std::vector<Viewport*> viewports;
};

#endif /* RENDERER_H_ */
