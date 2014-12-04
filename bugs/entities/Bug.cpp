/*
 * Bug.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#include "Bug.h"
#include "../neuralnet/Network.h"
#include "../genetics/Gene.h"
#include "../genetics/GeneDefinitions.h"
#include "../math/math2D.h"

Bug::Bug(Genome const &genome)
	: genome(genome)
	, neuralNet(new NeuralNet())
{
}

Bug::~Bug() {
}

Bug* Bug::newBasicBug() {
	Genome g;

	Gene::GeneData data;
	data.gene_command.command = GENE_DEV_GROW;
	data.gene_command.angle = PI * 3.f/2;
	data.gene_command.part_type = GENE_PART_BONE;
	data.gene_command.location = // what if a node has more than 2 children?

	g.second = g.first; // make a duplicate of all genes into the second chromosome

	return new Bug(g);
}
