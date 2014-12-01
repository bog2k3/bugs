/*
 * Bug.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#include "Bug.h"
#include "../neuralnet/Network.h"

Bug::Bug(Genome genome)
	: genome(genome)
	, neuralNet(new NeuralNet())
{
}

Bug::~Bug() {
}

