/*
 * GenomeFitness.cpp
 *
 *  Created on: Jul 17, 2018
 *      Author: bog
 */

#include "GenomeFitness.h"
#include "../entities/Bug/Bug.h"
#include "../body-parts/BodyPart.h"

float GenomeFitness::compute(Bug const& b) {
	float fitness = 0;
	const float max_fitness = 4.f; // +1 for egglayer, +1 for mouth, +2 for noses
	int noses = 0;
	bool hasMouth = false;
	bool hasEggLayer = false;
	auto &bparts = b.getBodyParts();
	if (bparts.size() == 0)
		return 0;
	bparts[0]->applyPredicateGraph([&](auto b) {
		switch (b->getType()) {
		case BodyPartType::EGGLAYER:
			if (!hasEggLayer) {
				fitness += 1;
				hasEggLayer = true;
			}
			break;
		case BodyPartType::MOUTH:
			if (!hasMouth) {
				fitness += 1;
				hasMouth = true;
			}
			break;
		case BodyPartType::SENSOR_PROXIMITY:
			if (noses++ < 2) {			// +1 for first nose, +1 for second, and none for anything more
				fitness += 1;
			}
			break;
		default:
			break;
		}
		return fitness >= max_fitness; // stop when max_fitness is reached
	});
	float fatRatio = b.getTotalFatMass() / b.getTotalMass();
	float idealFatRatio = 0.33f;
	float fatFitnessScore = (fatRatio > idealFatRatio ? 1.f / fatRatio : fatRatio) / idealFatRatio;	// max score when fatRatio is ideal, less if it's greater or smaller

	return fitness + fatFitnessScore;
}
