/*
 * LabelEntity.cpp
 *
 *  Created on: May 18, 2017
 *      Author: bog
 */

#include "LabelEntity.h"

LabelEntity::LabelEntity(std::string text, float size, ViewportCoord pos, float z, glm::vec3 color, std::set<std::string> viewportFilter)
	: label_(std::make_unique<Label>(text, pos, z, size, color, viewportFilter))
	, filter_(viewportFilter)
{
	label_->drawFrame = false;
}

LabelEntity::~LabelEntity() {
}

void LabelEntity::draw(RenderContext const&) {
	label_->draw();
}

