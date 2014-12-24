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
	const Viewport* viewport;
	Shape2D* shape;
	GLText* text;

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
