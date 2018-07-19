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
#include "GenomeFitness.h"
#include "MotorFitness.h"

#include <boglfw/World.h>
#include <boglfw/utils/parallel.h>
#include <boglfw/Infrastructure.h>

#include <algorithm>
#include <numeric>

Researcher::Researcher(std::string genomesPath)
	: genomesPath_(genomesPath)
{
}

void Researcher::saveGenomes() {

}

// load genomes, set target population and recombinationRatio - the fraction of targetPopulation that will be filled
// with new genomes created by recombining previous generation genomes (the rest will be prev gen genomes which are simply mutated)
void Researcher::initialize(int targetPopulation, float recombinationRatio, int motorSampleFrames, int randomGenomeLength) {
	targetPopulation_ = targetPopulation;
	recombinationRatio_ = recombinationRatio;
	motorSampleFrames_ = motorSampleFrames;
	randomGenomeLength_ = randomGenomeLength;
	loadGenomes();
	fillUpPopulation();
}

// perform a research iteration
void Researcher::iterate(float timeStep) {
	LOGLN("RESEARCH ITERATION ---------------------------------------------------------");
	MTVector<Bug*> bugs(genomes_.size());
	parallel_for(genomes_.begin(), genomes_.end(), Infrastructure::getThreadPool(), [this, timeStep, &bugs](auto &gp) {
		Genome& g = gp.first;
		Bug* b = new Bug(g, BodyConst::initialEggMass*2, {0,0}, {0,0}, 0);
		bugs.push_back(b);
		while (b->isInEmbryonicDevelopment())
			b->update(1.f);	// use 1 second step to bypass gene decode frequency delay in ribosome
		float &fitness = gp.second;
		fitness = 0;
		fitness = GenomeFitness::compute(*b);
		fitness += MotorFitness::compute(*b, motorSampleFrames_, timeStep);
	});
	// execute deferred tasks
	World::getInstance().update(0);
	// delete bugs
	for (auto b : bugs)
		delete b;
	bugs.clear();

	// sort by decreasing fitness
	std::sort(genomes_.begin(), genomes_.end(), [](auto &g1, auto &g2) {
		return g1.second > g2.second;
	});

	printIterationStats();

	// try to recombine recombinationRatio_ * targetPopulation_ matching genome pairs
	auto newGenomes = doRecombination();

	// select best genomes until targetPopulation_ is reached again
	selectBest(newGenomes);

	genomes_.swap(newGenomes);
}

void Researcher::loadGenomes() {
	// load as many genomes as needed or available from genomesPath_ directory
}

void Researcher::fillUpPopulation() {
	// generate random genomes until reaching the target population
	while (genomes_.size() < targetPopulation_)
		genomes_.push_back({GenomeGenerator::createRandom(randomGenomeLength_), 0.f});
}

decltype(Researcher::genomes_) Researcher::doRecombination() {
	decltype(genomes_) newGenomes;
	uint required = recombinationRatio_ * targetPopulation_;
	uint tries = 0;
	uint maxTries = genomes_.size();

	while (tries++ <= maxTries && newGenomes.size() < required) {
		uint i1 = biasedRandomSelect();
		uint i2 = biasedRandomSelect();

		Chromosome c1 = GeneticOperations::meyosis(genomes_[i1].first);
		Chromosome c2 = GeneticOperations::meyosis(genomes_[i2].first);

		if (c1.isGeneticallyCompatible(c2)) {
			auto newGenome = Genome{c1, c2};
			auto newFitness = (genomes_[i1].second + genomes_[i2].second) / 2;	// average fitness
			newGenomes.push_back({newGenome, newFitness});
		}
	}

	return newGenomes;
}

void Researcher::selectBest(decltype(genomes_) &out) {
	while (out.size() < targetPopulation_) {
		out.push_back(genomes_[biasedRandomSelect()]);
		// perform mutations:
		GeneticOperations::alterChromosome(out.back().first.first);
		GeneticOperations::alterChromosome(out.back().first.second);
	}
}

uint Researcher::biasedRandomSelect() {
	// each genome gets a chance to be selected proportional to its fitness squared
	// normalize chances to make them sum up to 1.0
	double total = std::accumulate(genomes_.begin(), genomes_.end(), 0.f, [](double t, auto &g) {
		return t + sqr(g.second);
	});
	double dice = randd();
	double floor = 0;
	uint selected = 0;
	for (uint i=0; i<genomes_.size(); i++) {
		auto &g = genomes_[i];
		if (dice - floor < (sqr(g.second) / total)) {
			selected = i;
			break;
		}
		floor += g.second;
	}
	return selected;
}

void Researcher::printIterationStats() {
	LOG("Iteration best fitnesses: ");
	int n=10;
	int printed = 0;
	for (int i=0; i<n; i++) {
		if (genomes_[i].second == 0)
			break;
		printed++;
		LOGNP(genomes_[i].second << ",   ");
	}
	if (!printed)
		LOGNP("nothing yet...");
	LOGLN("");
}

void Researcher::printStatistics() {

}
