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
#include "../genetics/Ribosome.h"

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
void Researcher::initialize(int targetPopulation, float recombinationRatio, float renewRatio, int motorSampleFrames, int randomGenomeLength) {
	targetPopulation_ = targetPopulation;
	recombinationRatio_ = recombinationRatio;
	renewRatio_ = renewRatio;
	motorSampleFrames_ = motorSampleFrames;
	randomGenomeLength_ = randomGenomeLength;
	loadGenomes();
	fillUpPopulation();
}

// perform a research iteration
void Researcher::iterate(float timeStep) {
	LOGLN("RESEARCH ITERATION -----------------------------------------------------------------------------------------------");
	MTVector<Bug*> bugs(genomes_.size());
	parallel_for(genomes_.begin(), genomes_.end(), Infrastructure::getThreadPool(), [this, timeStep, &bugs](auto &gp) {
		Genome& g = gp.first;
		Bug* b = new Bug(g, BodyConst::initialEggMass*2, {0,0}, {0,0}, 0);
		bugs.push_back(b);
		b->getRibosome()->setResearchModeOn();
		while (b->isInEmbryonicDevelopment() && !b->getRibosome()->isPreFinalStep())
			b->update(1.f);	// use 1 second step to bypass gene decode frequency delay in ribosome
	});
	// execute deferred tasks
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0);
	for (auto b : bugs)
		b->update(1.f); // last update to finalize ribosome stuff
	// another deferred tasks execution:
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0);

	// compute fitnesses
	MTVector<std::remove_reference<decltype(genomes_[0])>::type> updatedGenomes(genomes_.size());
	parallel_for(bugs.begin(), bugs.end(), Infrastructure::getThreadPool(), [this, &updatedGenomes, &timeStep] (auto b) {
		float fitness = 0;
		fitness += GenomeFitness::compute(*b);
		fitness += MotorFitness::compute(*b, motorSampleFrames_, timeStep);
		updatedGenomes.push_back({b->getGenome(), fitness});
		// kill bug
		b->kill();
	});
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0); // performed queued die events and stuff

	for (auto b : bugs) {
		b->destroy();
	}
	bugs.clear();
	// allow world to clean up entities
	while(World::getInstance().hasQueuedDeferredActions())
		World::getInstance().update(0);

	// copy back the genomes with the updated fitness scores
	genomes_.clear();
	for (auto &g : updatedGenomes)
		genomes_.push_back(g);

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

	// refresh population by adding some new random genomes
	fillUpPopulation();
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
	uint required = max(1.f, recombinationRatio_ * targetPopulation_);
	uint tries = 0;
	uint maxTries = genomes_.size();

	while (tries++ <= maxTries && newGenomes.size() < required) {
		uint i1 = biasedRandomSelect();
		uint i2 = biasedRandomSelect();

		Chromosome c1 = GeneticOperations::meyosis(genomes_[i1].first);
		Chromosome c2 = GeneticOperations::meyosis(genomes_[i2].first);

		if (c1.isGeneticallyCompatible(c2))
			newGenomes.push_back({Genome{c1, c2}, 0});
	}

	return newGenomes;
}

void Researcher::selectBest(decltype(genomes_) &out) {
	std::set<uint> selected;
	uint roomForNew = max(1.f, renewRatio_ * targetPopulation_);
	while (out.size() < targetPopulation_ - roomForNew) {
		uint index = biasedRandomSelect();
		out.push_back(genomes_[index]);
		genomes_.erase(genomes_.begin() + index); // in order to avoid duplicates
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
	for (int i=0; i<n && i<genomes_.size(); i++) {
		if (genomes_[i].second == 0)
			break;
		printed++;
		LOGNP(genomes_[i].second << ",   ");
	}
	if (!printed)
		LOGNP("nothing yet...");
	LOGNP("\n");
}

void Researcher::printStatistics() {

}
