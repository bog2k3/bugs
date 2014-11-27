/*
 * Shape2D.h
 *
 *  Created on: Nov 14, 2014
 *      Author: bogdan
 */

#ifndef RENDEROPENGL_SHAPE2D_H_
#define RENDEROPENGL_SHAPE2D_H_

#include "IRenderable.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

class Renderer;
class Viewport;

class Shape2D : public IRenderable {
public:
	Shape2D(Renderer* renderer);
	virtual ~Shape2D();

	void drawLine(glm::vec2 point1, glm::vec2 point2, float z, glm::vec3 rgb);
	void drawRectangle(glm::vec2 pos, float z, glm::vec2 size, float rotation, glm::vec3 rgb);
	void drawPolygon(glm::vec2 *verts, int nVerts, float z, glm::vec3 rgb);

private:
	void render(Viewport* vp);
	void purgeRenderQueue();

	struct s_lineVertex {
		glm::vec3 pos;	// position X,Y,Z
		glm::vec3 rgb; 	// color
	};
	std::vector<s_lineVertex> buffer;
	std::vector<unsigned short> indices;
	unsigned shaderProgram;
	unsigned indexPos;
	unsigned indexColor;
	unsigned indexMatViewProj;
};

#endif /* RENDEROPENGL_SHAPE2D_H_ */