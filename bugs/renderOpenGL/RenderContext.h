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

class RenderContext {
public:
	Shape2D* shape;
	const Viewport* viewport;
	GLText* text;

	RenderContext()
		: shape(nullptr)
		, viewport(nullptr)
		, text(nullptr) {
	}

	RenderContext(Shape2D* shape, Viewport* vp, GLText* text)
		: shape(shape), viewport(vp), text(text) {
	}
};

#endif /* RENDEROPENGL_RENDERCONTEXT_H_ */
