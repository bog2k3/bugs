/*
 * RenderContext.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bogdan
 */

#ifndef RENDEROPENGL_RENDERCONTEXT_H_
#define RENDEROPENGL_RENDERCONTEXT_H_

class Shape2D;
class Viewport;
class GLText;

struct RenderLayers {
	bool physics = true;	// true to draw physics debug data - physics objects/joints, etc
	bool bodyDebug = true;	// true to draw body debug data - muscles, parts inside zygote, etc
	bool bugDebug = true;	// true to draw bug debug data - bugID, etc
};

class RenderContext {
public:
	const Viewport* const viewport;
	Shape2D* const shape;
	GLText* const text;

	RenderLayers enabledLayers;

	RenderContext()
		: viewport(nullptr)
		, shape(nullptr)
		, text(nullptr) {
	}

	RenderContext(Viewport* vp, Shape2D* shape, GLText* text)
		: viewport(vp), shape(shape), text(text) {
	}
};

#endif /* RENDEROPENGL_RENDERCONTEXT_H_ */
