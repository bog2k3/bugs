/*
 * Rectangle.h
 *
 *  Created on: Oct 29, 2014
 *      Author: bog
 */

#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <vector>

class IRenderer;

class Rectangle {
public:
	static void initialize(IRenderer* renderer);
	static void draw(float center_x, float center_y, float z, float width, float height, float rotation,
			float red, float green, float blue);

private:
	Rectangle() {
	}
	static bool initialized;

	static void render();

	struct s_rectangle {
		float pos[3];	// position X,Y,Z
		float rgb[3]; 	// color
	};
	static std::vector<s_rectangle> buffer;
	static GLuint shaderProgram;
	static GLuint indexPos;
	static GLuint indexColor;
	static GLuint indexMatViewProj;
	static IRenderer* renderer;
};

#endif /* RECTANGLE_H_ */
