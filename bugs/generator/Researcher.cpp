/*
 * Researcher.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#include "Researcher.h"
#include "../entities/Bug/Bug.h"
#include "../body-parts/BodyConst.h"
#include "GenomeGenerator.h"

Researcher::Researcher(std::string genomesPath)
	: genomesPath_(genomesPath)
{
}

void Researcher::saveGenomes() {

}

// load genomes, set target population and recombinationRatio - the fraction of targetPopulation that will be filled
// with new genomes created by recombining previous generation genomes (the rest will be prev gen genomes wich are simply mutated)
void Researcher::initialize(int targetPopulation, float recombinationRatio, int motorSampleFrames) {
	targetPopulation_ = targetPopulation;
	recombinationRatio_ = recombinationRatio;
	motorSampleFrames_ = motorSampleFrames;
	loadGenomes();
	fillUpPopulation();
}

// perform a research iteration
void Researcher::iterate(float timeStep) {
	for (auto &gp : genomes_) {
		Genome& g = gp.first;
		float &fitness = gp.second;
		Bug b(g, BodyConst::initialEggMass*2, {0}, {0}, 0);
		while (b.isInEmbryonicDevelopment())
			b.update(1.f);	// use 1 second step to bypass gene decode frequency delay in ribosome
		fitness = GenomeFitness::compute(b);
		fitness += MotorFitness::compute(b, motorSampleFrames_, timeStep);
	}
}

void Researcher::loadGenomes() {
	// load as many genomes as needed or available from genomesPath_ directory
}

void Researcher::fillUpPopulation() {
	// generate random genomes until reaching the target population
	while (genomes_.size() < targetPopulation_)
		genomes_.push_back({GenomeGenerator::createRandom(), 0.f});
}
