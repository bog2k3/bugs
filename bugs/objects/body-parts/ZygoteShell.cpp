/*
 * ZygoteShell.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "ZygoteShell.h"
#include <glm/vec2.hpp>

ZygoteShell::ZygoteShell(float size, PhysicsProperties props)
	: BodyPart(nullptr, BODY_PART_ZYGOTE_SHELL, props)
{
}

ZygoteShell::~ZygoteShell() {
	// TODO Auto-generated destructor stub
}

