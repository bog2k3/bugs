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
#include <string>

class Renderer;
class Viewport;

// renders 2D shapes in viewport space
class Shape2D : public IRenderable {
public:
	static Shape2D* get();
	virtual ~Shape2D() override;
	static void init(Renderer* renderer);

	// restricts rendering of following commands to only one viewport
	void setViewportFilter(std::string viewportName);
	// resets the viewport filter
	void resetViewportFilter();

	// draw a single line segment
	void drawLine(ViewportCoord point1, ViewportCoord point2, float z, glm::vec3 rgb);
	void drawLine(ViewportCoord point1, ViewportCoord point2, float z, glm::vec4 rgba);
	// draw a list of separate lines (pairs of two vertices)
	void drawLineList(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb);
	void drawLineList(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba);
	// draw a line strip (connected lines)
	void drawLineStrip(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb);
	void drawLineStrip(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba);
	// draw a rectangle; pos is the top-left position
	void drawRectangle(ViewportCoord pos, float z, ViewportCoord size, glm::vec3 rgb);
	void drawRectangle(ViewportCoord pos, float z, ViewportCoord size, glm::vec4 rgba);
	// draw a rectangle; pos is the center position
	void drawRectangleCentered(ViewportCoord pos, float z, ViewportCoord size, glm::vec3 rgb);
	void drawRectangleCentered(ViewportCoord pos, float z, ViewportCoord size, glm::vec4 rgba);
	// draw a filled rectangle; pos is the center position
	void drawRectangleFilled(ViewportCoord pos, float z, ViewportCoord size, glm::vec3 rgb);
	void drawRectangleFilled(ViewportCoord pos, float z, ViewportCoord size, glm::vec4 rgba);
	// draw a polygon
	void drawPolygon(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb);
	void drawPolygon(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba);
	// draw a filled polygon
	void drawPolygonFilled(ViewportCoord verts[], int nVerts, float z, glm::vec3 rgb);
	void drawPolygonFilled(ViewportCoord verts[], int nVerts, float z, glm::vec4 rgba);
	// draw a circle
	void drawCircle(ViewportCoord pos, float radius, float , int nSides, glm::vec3 rgb);
	void drawCircle(ViewportCoord pos, float radius, float , int nSides, glm::vec4 rgba);
	// draw a filled circle
	void drawCircleFilled(ViewportCoord pos, float radius, float , int nSides, glm::vec3 rgb);
	void drawCircleFilled(ViewportCoord pos, float radius, float , int nSides, glm::vec4 rgba);

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

	struct s_batch {
		size_t offset;	// offset in index buffer
		size_t length;	// number of indices
		std::string viewportFilter;
	};
	std::vector<s_batch> batches_;

	// triangle buffers
	std::vector<s_lineVertex> bufferTri_;
	std::vector<unsigned short> indicesTri_;

	std::string viewportFilter_ {""};
	unsigned lineShaderProgram_;
	unsigned indexPos_;
	unsigned indexColor_;
	unsigned indexMatViewport_;
};

#endif /* RENDEROPENGL_SHAPE2D_H_ */
