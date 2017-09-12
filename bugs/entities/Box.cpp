/*
 * Cube.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: bog
 */

#include "Box.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/MeshRenderer.h"

Box::Box(float width, float height, float depth, glm::vec3 offset) {
	mesh_.createBox(offset, width, height, depth);
}

Box::~Box() {
}

void Box::update(float dt) {
	//body_.update(dt);
}

void Box::draw(RenderContext const& ctx) {
	MeshRenderer::get()->renderMesh(mesh_, glm::mat4(1)/*body_.getTransformation(physics::DynamicBody::TransformSpace::World)*/);
}
