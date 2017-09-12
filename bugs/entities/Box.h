/*
 * Cube.h
 *
 *  Created on: Apr 24, 2017
 *      Author: bog
 */

#ifndef ENTITIES_BOX_H_
#define ENTITIES_BOX_H_

#include "Entity.h"
#include "../renderOpenGL/Mesh.h"
//#include "../physics/DynamicBody.h"

class Box: public Entity {
public:
	Box(float width, float height, float depth, glm::vec3 offset = glm::vec3(0));
	virtual ~Box();

	EntityType getEntityType() const override { return EntityType::BOX; }
	FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE | FunctionalityFlags::UPDATABLE; }

	void update(float dt) override;
	void draw(RenderContext const& ctx) override;

	//physics::DynamicBody* body() { return &body_; }

private:
	Mesh mesh_;
	//physics::DynamicBody body_;
};

#endif /* ENTITIES_BOX_H_ */
