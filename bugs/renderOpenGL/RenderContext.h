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
	Shape2D* const shape;
	const Viewport* const viewport;
	GLText* const text;

	RenderContext()
		: shape(nullptr)
		, viewport(nullptr)
		, text(nullptr) {
	}

	RenderContext(Shape2D* shape, Viewport* vp, GLText* text)
		: shape(shape), viewport(vp), text(text) {
	}

	RenderContext(RenderContext const& orig) = default;
	RenderContext& operator =(RenderContext const& orig) {
		*this = RenderContext(orig);
		return *this;
	}
};

#endif /* RENDEROPENGL_RENDERCONTEXT_H_ */
