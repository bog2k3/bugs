/*
 * Shape3D.h
 *
 *  Created on: Sep 12, 2017
 *      Author: bogdan
 */

#ifndef RENDEROPENGL_SHAPE3D_H_
#define RENDEROPENGL_SHAPE3D_H_

#include "IRenderable.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <set>
#include <string>

class Renderer;
class Viewport;

// renders 3D shapes in world space
class Shape3D : public IRenderable {
public:
	static Shape3D* get();
	virtual ~Shape3D() override;
	static void init(Renderer* renderer);

	// draw a single line segment
	void drawLine(glm::vec3 point1, glm::vec3 point2, glm::vec3 rgb);
	void drawLine(glm::vec3 point1, glm::vec3 point2, glm::vec4 rgba);
	// draw a list of separate lines (pairs of two vertices)
	void drawLineList(glm::vec3 verts[], int nVerts, glm::vec3 rgb);
	void drawLineList(glm::vec3 verts[], int nVerts, glm::vec4 rgba);
	// draw a line strip (connected lines)
	void drawLineStrip(glm::vec3 verts[], int nVerts, glm::vec3 rgb);
	void drawLineStrip(glm::vec3 verts[], int nVerts, glm::vec4 rgba);
	// draw a rectangle; pos is the top-left position
	void drawRectangleXOY(glm::vec2 pos, glm::vec2 size, glm::vec3 rgb);
	void drawRectangleXOY(glm::vec2 pos, glm::vec2 size, glm::vec4 rgba);
	// draw a rectangle; pos is the center position
	void drawRectangleXOYCentered(glm::vec2 pos, glm::vec2 size, float rotation, glm::vec3 rgb);
	void drawRectangleXOYCentered(glm::vec2 pos, glm::vec2 size, float rotation, glm::vec4 rgba);

	// draw a polygon
	void drawPolygon(glm::vec3 verts[], int nVerts, glm::vec3 rgb);
	void drawPolygon(glm::vec3 verts[], int nVerts, glm::vec4 rgba);

	// draw a circle
	void drawCircleXOY(glm::vec2 pos, float radius, int nSides, glm::vec3 rgb);
	void drawCircleXOY(glm::vec2 pos, float radius, int nSides, glm::vec4 rgba);

protected:
	Shape3D(Renderer* renderer);

private:
	void render(Viewport* vp) override;
	void purgeRenderQueue() override;
	void unload() override;

	struct s_lineVertex {
		glm::vec3 pos;
		glm::vec4 rgba; 	// color
	};
	// line buffers
	std::vector<s_lineVertex> buffer_;
	std::vector<unsigned short> indices_;

	unsigned lineShaderProgram_;
	unsigned indexPos_;
	unsigned indexColor_;
	unsigned indexMatProjView_;
};

#endif /* RENDEROPENGL_SHAPE3D_H_ */
