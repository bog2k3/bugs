/*
 * FoodDispenser.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#include "FoodDispenser.h"

FoodDispenser::FoodDispenser(glm::vec2 position, float direction) {
	// TODO create fixture and initialize parameters.
}

FoodDispenser::~FoodDispenser() {
	// TODO Auto-generated destructor stub
}

void FoodDispenser::draw(RenderContext& ctx) {

}

template<> void update(FoodDispenser*& disp, float dt) {
	disp->update(dt);
}

void FoodDispenser::update(float dt) {
	//TODO here it squirts food chunks
}
