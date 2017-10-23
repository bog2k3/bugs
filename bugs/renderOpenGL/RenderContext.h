/*
 * RenderContext.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bogdan
 */

#ifndef RENDEROPENGL_RENDERCONTEXT_H_
#define RENDEROPENGL_RENDERCONTEXT_H_

#include <functional>
#include <vector>

class Shape2D;
class Viewport;
class GLText;
class MeshRenderer;

struct RenderLayers {
	bool physics = true;	// true to draw physics debug data - physics objects/joints, etc
	bool bodyDebug = true;	// true to draw body debug data - muscles, parts inside zygote, etc
	bool bugDebug = true;	// true to draw bug debug data - bugID, etc
	bool meshes = true;	// 3D meshes
	bool osd = true;	// 2D on-screen-display
};

using deferredCallback = std::function<void(Viewport* vp)>;

class RenderContext {
public:
	RenderLayers enabledLayers;

	RenderContext() = default;

	// defers the drawing to a function which will be called on a per-viewport basis
	void defer(deferredCallback perViewportCallback) const { deferred_.push_back(perViewportCallback); }

private:
	mutable std::vector<deferredCallback> deferred_;
	friend class Renderer;
};

#endif /* RENDEROPENGL_RENDERCONTEXT_H_ */
