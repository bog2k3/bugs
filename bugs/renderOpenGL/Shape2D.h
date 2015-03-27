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
#include <glm/vec4.hpp>
#include <vector>

class Renderer;
class Viewport;

class Shape2D : public IRenderable {
public:
	Shape2D(Renderer* renderer);
	virtual ~Shape2D();

	// set up the next draw calls to be executed in viewport space or world space
	void setViewportSpaceDraw(bool value) { viewportSpaceEnabled_ = value; }

	// draw a single line segment
	void drawLine(glm::vec2 const &point1, glm::vec2 const &point2, float z, glm::vec3 const &rgb);
	void drawLine(glm::vec2 const &point1, glm::vec2 const &point2, float z, glm::vec4 const &rgba);
	// draw a list of separate lines (pairs of two vertices)
	void drawLineList(glm::vec2* verts, int nVerts, float z, glm::vec3 const &rgb);
	void drawLineList(glm::vec2* verts, int nVerts, float z, glm::vec4 const &rgba);
	// draw a line strip (connected lines)
	void drawLineStrip(glm::vec2* verts, int nVerts, float z, glm::vec3 const &rgb);
	void drawLineStrip(glm::vec2* verts, int nVerts, float z, glm::vec4 const &rgba);
	// draw a rectangle; pos is the top-left position
	void drawRectangle(glm::vec2 const &pos, float z, glm::vec2 const &size, glm::vec3 const &rgb);
	void drawRectangle(glm::vec2 const &pos, float z, glm::vec2 const &size, glm::vec4 const &rgba);
	// draw a rectangle; pos is the center position
	void drawRectangleCentered(glm::vec2 const &pos, float z, glm::vec2 const &size, float rotation, glm::vec3 const &rgb);
	void drawRectangleCentered(glm::vec2 const &pos, float z, glm::vec2 const &size, float rotation, glm::vec4 const &rgba);
	// draw a filled rectangle; pos is the center position
	void drawRectangleFilled(glm::vec2 const &pos, float z, glm::vec2 const &size, glm::vec3 const &rgb);
	void drawRectangleFilled(glm::vec2 const &pos, float z, glm::vec2 const &size, glm::vec4 const &rgba);
	// draw a polygon
	void drawPolygon(glm::vec2 *verts, int nVerts, float z, glm::vec3 const &rgb);
	void drawPolygon(glm::vec2 *verts, int nVerts, float z, glm::vec4 const &rgba);
	// draw a filled polygon
	void drawPolygonFilled(glm::vec2 *verts, int nVerts, float z, glm::vec3 const &rgb);
	void drawPolygonFilled(glm::vec2 *verts, int nVerts, float z, glm::vec4 const &rgba);
	// draw a circle
	void drawCircle(glm::vec2 const &pos, float radius, float , int nSides, glm::vec3 const &rgb);
	void drawCircle(glm::vec2 const &pos, float radius, float , int nSides, glm::vec4 const &rgba);
	// draw a filled circle
	void drawCircleFilled(glm::vec2 const &pos, float radius, float , int nSides, glm::vec3 const &rgb);
	void drawCircleFilled(glm::vec2 const &pos, float radius, float , int nSides, glm::vec4 const &rgba);

private:
	void render(Viewport* vp) override;
	void purgeRenderQueue() override;

	struct s_lineVertex {
		glm::vec3 pos;	// position X,Y,Z
		glm::vec4 rgba; 	// color
	};
	// line buffers for world-space and viewport-space rendering:
	std::vector<s_lineVertex> buffer;
	std::vector<unsigned short> indices;
	std::vector<s_lineVertex> bufferVPSP;	// vertex buffer for ViewPortSPace rendering
	std::vector<unsigned short> indicesVPSP; // index buffer for ViewPortSPace rendering
	// triangle buffers for world-space and viewport-space rendering:
	std::vector<s_lineVertex> bufferTri;
	std::vector<unsigned short> indicesTri;
	std::vector<s_lineVertex> bufferTriVPSP;	// triangle vertex buffer for ViewPortSPace rendering
	std::vector<unsigned short> indicesTriVPSP; // triangle index buffer for ViewPortSPace rendering
	unsigned lineShaderProgram;
	unsigned indexPos;
	unsigned indexColor;
	unsigned indexMatViewProj;
	bool viewportSpaceEnabled_ = false;
};

#endif /* RENDEROPENGL_SHAPE2D_H_ */
