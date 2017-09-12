/*
 * Shape2D.h
 *
 *  Created on: Nov 14, 2014
 *      Author: bogdan
 */

#ifndef RENDEROPENGL_SHAPE2D_H_
#define RENDEROPENGL_SHAPE2D_H_

#include "IRenderable.h"
#include "ViewportCoord.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <set>
#include <string>

class Renderer;
class Viewport;

// renders 2D shapes in viewport space
class Shape2D : public IRenderable {
public:
	static Shape2D* get();
	virtual ~Shape2D() override;
	static void init(Renderer* renderer);

	// draw a single line segment
	void drawLine(ViewportCoord point1, ViewportCoord point2, float z, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawLine(ViewportCoord point1, ViewportCoord point2, float z, glm::vec4 rgba, std::set<std::string> viewportFilter = {});
	// draw a list of separate lines (pairs of two vertices)
	void drawLineList(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawLineList(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba, std::set<std::string> viewportFilter = {});
	// draw a line strip (connected lines)
	void drawLineStrip(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawLineStrip(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba, std::set<std::string> viewportFilter = {});
	// draw a rectangle; pos is the top-left position
	void drawRectangle(ViewportCoord pos, float z, ViewportCoord size, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawRectangle(ViewportCoord pos, float z, ViewportCoord size, glm::vec4 rgba, std::set<std::string> viewportFilter = {});
	// draw a rectangle; pos is the center position
	void drawRectangleCentered(ViewportCoord pos, float z, ViewportCoord size, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawRectangleCentered(ViewportCoord pos, float z, ViewportCoord size, glm::vec4 rgba, std::set<std::string> viewportFilter = {});
	// draw a filled rectangle; pos is the center position
	void drawRectangleFilled(ViewportCoord pos, float z, ViewportCoord size, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawRectangleFilled(ViewportCoord pos, float z, ViewportCoord size, glm::vec4 rgba, std::set<std::string> viewportFilter = {});
	// draw a polygon
	void drawPolygon(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawPolygon(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba, std::set<std::string> viewportFilter = {});
	// draw a filled polygon
	void drawPolygonFilled(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawPolygonFilled(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba, std::set<std::string> viewportFilter = {});
	// draw a circle
	void drawCircle(ViewportCoord pos, float radius, float , int nSides, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawCircle(ViewportCoord pos, float radius, float , int nSides, glm::vec4 rgba, std::set<std::string> viewportFilter = {});
	// draw a filled circle
	void drawCircleFilled(ViewportCoord pos, float radius, float , int nSides, glm::vec3 rgb, std::set<std::string> viewportFilter = {});
	void drawCircleFilled(ViewportCoord pos, float radius, float , int nSides, glm::vec4 rgba, std::set<std::string> viewportFilter = {});

protected:
	Shape2D(Renderer* renderer);

private:
	void render(Viewport* vp) override;
	void purgeRenderQueue() override;
	void unload() override;

	struct s_lineVertex {
		ViewportCoord pos;
		float z;
		glm::vec4 rgba; 	// color
	};
	// line buffers
	std::vector<s_lineVertex> buffer_;
	std::vector<unsigned short> indices_;
	std::vector<std::set<std::string>> viewportFiltersLine_;
	// triangle buffers
	std::vector<s_lineVertex> bufferTri_;
	std::vector<unsigned short> indicesTri_;
	std::vector<std::set<std::string>> viewportFiltersTri_;
	unsigned lineShaderProgram_;
	unsigned indexPos_;
	unsigned indexColor_;
	unsigned indexMatViewport_;
};

#endif /* RENDEROPENGL_SHAPE2D_H_ */
