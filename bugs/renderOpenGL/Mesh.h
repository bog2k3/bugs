/*
 * Mesh.h
 *
 *  Created on: Apr 24, 2017
 *      Author: bog
 */

#ifndef RENDEROPENGL_MESH_H_
#define RENDEROPENGL_MESH_H_

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

class Mesh {
public:
	Mesh();
	virtual ~Mesh();

	void createBox(glm::vec3 center, float width, float height, float depth);
	void createSphere(glm::vec3 center, float radius, int detail=10);

	unsigned getVertexBuffer() const { return vertexBuffer_; }
	unsigned getIndexBuffer() const { return indexBuffer_; }
	unsigned getVertexCount() const { return vertexCount_; }
	unsigned getIndexCount() const { return indexCount_; }

private:
	friend class MeshRenderer;

	unsigned vertexBuffer_ = 0;
	unsigned indexBuffer_ = 0;
	unsigned vertexCount_ = 0;
	unsigned indexCount_ = 0;

	bool dirty_ = true;

	struct s_Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 UV1;
		glm::vec3 color;
	};

	std::vector<s_Vertex> vertices_;
	std::vector<uint16_t> indices_;

	bool isDirty() const { return dirty_; }
	void commitChanges();
};

#endif /* RENDEROPENGL_MESH_H_ */
