/*
 * Researcher.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#include "Researcher.h"
#include "../entities/Bug/Bug.h"
#include "../body-parts/BodyConst.h"
#include "../genetics/Ribosome.h"
#include "../serialization/GenomeSerialization.h"
#include "../serialization/separatedTextOutStream.h"
#include "GenomeGenerator.h"
#include "GenomeFitness.h"
#include "MotorFitness.h"

#include <boglfw/World.h>
#include <boglfw/utils/parallel.h>
#include <boglfw/Infrastructure.h>
#include <boglfw/utils/filesystem.h>

#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>

Researcher::Researcher(std::string genomesPath)
	: genomesPath_(genomesPath)
{
}

void Researcher::saveGenomes() {
	int i = 0;
	for (auto &g : genomes_) {
		std::stringstream ss;
		ss << genomesPath_ + "/genome-" << i++ << ".txt";
		std::ofstream f(ss.str());
		//SeparatedTextOutStream sf(f);
		f << g.second << g.first;
	}
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
	stats_.push_back({});
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

	for (auto &g : genomes_)
		stats_.back().fitness.push_back(g.second);

	// try to recombine recombinationRatio_ * targetPopulation_ matching genome pairs
	auto newGenomes = doRecombination();

	// select best genomes until targetPopulation_ is reached again
	selectBest(newGenomes);

	genomes_.swap(newGenomes);

	stats_.back().newRandom = targetPopulation_ - genomes_.size();
	// refresh population by adding some new random genomes
	fillUpPopulation();

	printIterationStats();
}

void Researcher::loadGenomes() {
	// load as many genomes as needed or available from genomesPath_ directory
	auto files = filesystem::getFiles(genomesPath_);
	for (auto &fn : files) {
		// load from file fn
		std::ifstream f(fn);
		float fitness;
		Genome g;
		f >> fitness >> g;

		genomes_.push_back({g, fitness});

		if (genomes_.size() == targetPopulation_)
			break;
	}
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

	stats_.back().recombinationTarget = required;

	while (tries++ <= maxTries && newGenomes.size() < required) {
		uint i1 = biasedRandomSelect(1.f, {});
		uint i2 = biasedRandomSelect(1.f, {});

		Chromosome c1 = GeneticOperations::meyosis(genomes_[i1].first);
		Chromosome c2 = GeneticOperations::meyosis(genomes_[i2].first);

		if (c1.isGeneticallyCompatible(c2)) {
			newGenomes.push_back({Genome{c1, c2}, 0});

			stats_.back().recombinationPairs.push_back({i1, i2});
		}
	}

	return newGenomes;
}

void Researcher::selectBest(decltype(genomes_) &out) {
	std::set<uint> selected;
	uint roomForNew = max(1.f, renewRatio_ * targetPopulation_);
	while (out.size() < targetPopulation_ - roomForNew) {
		uint index = biasedRandomSelect(1.f, selected);
		selected.insert(index);
		out.push_back(genomes_[index]);
		genomes_[index].second = 0; // in order to avoid duplicates
		// perform mutations on half of the genomes:
		if (randf() < 0.5) {
			GeneticOperations::alterChromosome(out.back().first.first);
			GeneticOperations::alterChromosome(out.back().first.second);
		}

		stats_.back().selected.push_back(index);
	}
}

// smaller than 1.0 values are more likely to also select from the ones with smaller fitnesses
// greater than 1.0 values are more likely to only select the highest fitnesses
uint Researcher::biasedRandomSelect(float steepness, std::set<uint> exclude) {
	// each genome gets a chance to be selected proportional to its fitness raised to "steepness" power
	// normalize chances to make them sum up to 1.0
	double total = 0;
	for (uint i=0; i<genomes_.size(); i++) {
		if (exclude.find(i) == exclude.end())
			total += pow(genomes_[i].second, steepness);
	}
	double dice = randd() * total;
	double floor = 0;
	uint selected = genomes_.size();
	for (uint i=0; i<genomes_.size(); i++) {
		if (exclude.find(i) != exclude.end())
			continue;
		auto &g = genomes_[i];
		if (dice - floor <= pow(g.second, steepness)) {
			selected = i;
			break;
		}
		floor += pow(g.second, steepness);
	}
	assertDbg(selected < genomes_.size());
	return selected;
}

void Researcher::printIterationStats() {
	LOG("Iteration best fitnesses: ");
	uint n=10;
	uint printed = 0;
	for (uint i=0; i<n && i<stats_.back().fitness.size(); i++) {
		if (stats_.back().fitness[i] == 0)
			break;
		printed++;
		LOGNP(stats_.back().fitness[i] << ",   ");
	}
	if (!printed)
		LOGNP("nothing yet...");
	LOGNP("\n");
	LOGLN("Recombined " << stats_.back().recombinationPairs.size() << " out of " << stats_.back().recombinationTarget << " pairs of genomes:");
	LOG("\t");
	for (auto &p : stats_.back().recombinationPairs) {
		LOGNP(p.first << " (f: " << stats_.back().fitness[p.first] << ") x " <<
				p.second << " (f: " << stats_.back().fitness[p.second] << ");  ");
	}
	LOGNP("\n");
	LOG("Selected genomes for next generation: ");
	for (auto i : stats_.back().selected)
		LOGNP(i << " (f: " << stats_.back().fitness[i] << "); ");
	LOGNP("\n");
	LOGLN("Fresh new genomes for next generation: " << stats_.back().newRandom);

	float avgFitness = std::accumulate(stats_.back().fitness.begin(), stats_.back().fitness.end(), 0.f);
	avgFitness /= stats_.back().fitness.size();
	LOGLN("Average iteration fitness: " << avgFitness);
}

void Researcher::printStatistics() {
	// TODO ...
}
