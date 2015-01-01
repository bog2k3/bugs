/*
 * ZygoteShell.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "ZygoteShell.h"
#include "../../math/math2D.h"
#include "../../renderOpenGL/Shape2D.h"
#include "../../renderOpenGL/RenderContext.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

const glm::vec3 debug_color(0.5f, 0.5f, 0.5f);

ZygoteShell::ZygoteShell(float size)
	: BodyPart(nullptr, BODY_PART_ZYGOTE_SHELL, std::make_shared<BodyPartInitializationData>())
{
	getInitializationData()->size = size;
}

ZygoteShell::~ZygoteShell() {
	// delete fixture
}

void ZygoteShell::draw(RenderContext& ctx) {
	if (committed_) {
		// nothing, physics draws
	} else {
		glm::vec3 transform = getWorldTransformation();
		glm::vec2 pos = vec3xy(transform);
		ctx.shape->drawCircle(pos, sqrtf(getInitializationData()->size / PI), 0, 12, debug_color);
		ctx.shape->drawLine(pos,
				pos + glm::rotate(glm::vec2(sqrtf(getInitializationData()->size / PI), 0), transform.z),
				0, debug_color);
	}
}
