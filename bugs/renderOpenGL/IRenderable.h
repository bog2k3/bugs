/*
 * IRenderable.h
 *
 *  Created on: Nov 9, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_IRENDERABLE_H_
#define RENDEROPENGL_IRENDERABLE_H_

class Viewport;

class IRenderable {
public:
	virtual ~IRenderable() {}
	/*
	 * This is called on the object at each frame, for each active viewport.
	 */
	virtual void render(Viewport* pCrtViewport) = 0;

	/**
	 * Called once per frame, after viewports are finished, to clear queued data
	 */
	virtual void purgeRenderQueue() = 0;

	// called once when the renderer is destroyed to release all resources associated with this renderable.
	virtual void unload() = 0;
};


#endif /* RENDEROPENGL_IRENDERABLE_H_ */
