/*
 * Infrastructure.cpp
 *
 *  Created on: Jun 23, 2016
 *      Author: bog
 */

#include "Infrastructure.h"
#include <thread>
#include <algorithm>

Infrastructure::Infrastructure()
	: threadPool_(std::max(1u, (std::thread::hardware_concurrency()*3u) / 2u)) {
}

void Infrastructure::shutDown_() {
	threadPool_.stop();
}

