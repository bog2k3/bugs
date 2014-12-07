/*
 * ZygoteShell.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "ZygoteShell.h"
#include <glm/vec2.hpp>

ZygoteShell::ZygoteShell(World* world, glm::vec2 position, float angle, bool dynamic, glm::vec2 velocity, float angularVelocity)
	: BodyPart(world, position, angle, velocity, angularVelocity, BODY_PART_ZYGOTE_SHELL)
{
}

ZygoteShell::~ZygoteShell() {
	// TODO Auto-generated destructor stub
}

