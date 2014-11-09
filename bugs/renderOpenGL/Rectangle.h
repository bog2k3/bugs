/*
 * Rectangle.h
 *
 *  Created on: Oct 29, 2014
 *      Author: bog
 */

#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include "IRenderable.h"

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <vector>

class Renderer;
class Viewport;

class Rectangle : public IRenderable {
public:
	Rectangle(Renderer* renderer);
	virtual ~Rectangle();
	void draw(float center_x, float center_y, float z, float width, float height, float rotation,
			float red, float green, float blue);

private:
	void render(Viewport* vp);
	void purgeRenderQueue();

	struct s_rectangle {
		float pos[3];	// position X,Y,Z
		float rgb[3]; 	// color
	};
	std::vector<s_rectangle> buffer;
	GLuint shaderProgram;
	GLuint indexPos;
	GLuint indexColor;
	GLuint indexMatViewProj;
};

#endif /* RECTANGLE_H_ */
