/*
 * Mesh.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: bog
 */

#include "Mesh.h"
#include <GL/glew.h>
#include <GL/gl.h>

Mesh::Mesh() {
	uint bufs[2];
	glGenBuffers(2, bufs);
	vertexBuffer_ = bufs[0];
	indexBuffer_ = bufs[1];
}

Mesh::~Mesh() {
	uint bufs[] { vertexBuffer_, indexBuffer_ };
	glDeleteBuffers(2, bufs);
}

void Mesh::createBox(glm::vec3 center, float width, float height, float depth) {
	float left = center.x - width * 0.5f;
	float right = center.x + width * 0.5f;
	float bottom = center.y - height * 0.5f;
	float top = center.y + height * 0.5f;
	float back = center.z - depth * 0.5f;
	float front = center.z + depth * 0.5f;
	glm::vec3 nBack(0, 0, -1);
	glm::vec3 nFront(0, 0, 1);
	glm::vec3 nLeft(-1, 0, 0);
	glm::vec3 nRight(1, 0, 0);
	glm::vec3 nTop(0, 1, 0);
	glm::vec3 nBottom(0, -1, 0);
	glm::vec3 white(1, 1, 1);
	vertices_.clear();
	vertices_.assign({
	// back face
		// #0 back bottom left
		{
			{left, bottom, back}, nBack, {0, 0}, white
		},
		// #1 back top left
		{
			{left, top, back}, nBack, {0, 1}, white
		},
		// #2 back top right
		{
			{right, top, back}, nBack, {1, 1}, white
		},
		// #3 back bottom right
		{
			{right, bottom, back}, nBack, {1, 0}, white
		},
	// top face
		// #4 back top left
		{
			{left, top, back}, nTop, {0, 0}, white
		},
		// #5 front top left
		{
			{left, top, front}, nTop, {0, 1}, white
		},
		// #6 front top right
		{
			{right, top, front}, nTop, {1, 1}, white
		},
		// #7 back top right
		{
			{right, top, back}, nTop, {1, 0}, white
		},
	// front face
		// #8 front top right
		{
			{right, top, front}, nFront, {0, 0}, white
		},
		// #9 front top left
		{
			{left, top, front}, nFront, {0, 1}, white
		},
		// #10 front bottom left
		{
			{left, bottom, front}, nFront, {1, 1}, white
		},
		// #11 front bottom right
		{
			{right, bottom, front}, nFront, {1, 0}, white
		},
	// bottom face
		// #12 front bottom left
		{
			{left, bottom, front}, nBottom, {0, 0}, white
		},
		// #13 back bottom left
		{
			{left, bottom, back}, nBottom, {0, 1}, white
		},
		// #14 back bottom right
		{
			{right, bottom, back}, nBottom, {1, 1}, white
		},
		// #15 front bottom right
		{
			{right, bottom, front}, nBottom, {1, 0}, white
		},
	// left face
		// #16 front bottom left
		{
			{left, bottom, front}, nLeft, {0, 0}, white
		},
		// #17 front top left
		{
			{left, top, front}, nLeft, {0, 1}, white
		},
		// #18 back top left
		{
			{left, top, back}, nLeft, {1, 1}, white
		},
		// #19 back bottom left
		{
			{left, bottom, back}, nLeft, {1, 0}, white
		},
	// right face
		// #20 back bottom right
		{
			{right, bottom, front}, nRight, {0, 0}, white
		},
		// #21 back top right
		{
			{right, top, front}, nRight, {0, 1}, white
		},
		// #22 front top right
		{
			{right, top, back}, nRight, {1, 1}, white
		},
		// #23 front bottom right
		{
			{right, bottom, back}, nRight, {1, 0}, white
		}
	});

#if (1)	// debug vertices with colors
	glm::vec3 c[] { {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0} };
	for (uint i=0; i<vertices_.size(); i++) {
		vertices_[i].color = c[i % (sizeof(c) / sizeof(c[0]))];
	}
#endif

	indices_.clear();
	indices_.assign({
		// back face
		0, 1, 2, 0, 2, 3,
		// top face
		4, 5, 6, 4, 6, 7,
		// front face
		8, 9, 10, 8, 10, 11,
		// bottom face
		12, 13, 14, 12, 14, 15,
		// left face
		16, 17, 18, 16, 18, 19,
		// right face
		20, 21, 22, 20, 22, 23
	});

	dirty_ = true;
}

void Mesh::createSphere(glm::vec3 center, float radius, int detail) {

}

void Mesh::commitChanges() {
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(vertices_[0]), vertices_.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	vertexCount_ = vertices_.size();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(indices_[0]), indices_.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	indexCount_ = indices_.size();

	dirty_ = false;
}
