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
#include "../genetics/constants.h"
#include "../genetics/Ribosome.h"
#include "../math/math2D.h"

const float DECODE_FREQUENCY = 1.f; // genes per second
const float DECODE_PERIOD = 1.f / DECODE_FREQUENCY; // seconds
const float ZYGOTE_ENERGY_DENSITY = 1000; // Joules per m^2

Bug::Bug(Genome const &genome, float zygoteSize)
	: genome(genome)
	, neuralNet(new NeuralNet())
	, ribosome(new Ribosome(this))
	, isAlive(true)
	, isDeveloping(true)
	, tRibosomeStep(0)
	, energy(zygoteSize * ZYGOTE_ENERGY_DENSITY)
	, scale(0.05f)
	, scaledEnergy(0)
{
	// pe masura ce se dezvolta, fiecare noua parte consuma energie.
	// energia disponibila in zigot si energia necesara dezvoltarii determina scala initiala.
}

Bug::~Bug() {
}

void Bug::update(float dt) {
	if (isAlive) {
		if (isDeveloping) {
			// developing embryo
			tRibosomeStep += dt;
			if (tRibosomeStep >= DECODE_PERIOD) {
				tRibosomeStep -= DECODE_PERIOD;
				isDeveloping = ribosome->step();
				if (!isDeveloping) {
					// delete embryo shell
				}
			}
		} else {
			// adult life
		}
	} else {
		// decaying
	}
}

Bug* Bug::newBasicBug() {
	Genome g;

	GeneCommand gc;
	// first bone:
	gc.command = GENE_DEV_GROW;
	gc.angle.set(PI * 3.f/2);
	gc.part_type = GENE_PART_BONE;
	gc.location.set(0);
	g.first.push_back(gc);

	// second bone:
	gc.location.set(0b1001);
	g.first.push_back(gc);

	// the gripper:
	gc.location.set(0b1001001001);
	gc.part_type = GENE_PART_GRIPPER;
	g.first.push_back(gc);

	// bone 1 muscle 1:
	gc.location.set(0b1001);
	gc.angle.set(PI);
	gc.part_type = GENE_PART_MUSCLE;
	g.first.push_back(gc);

	// bone 1 muscle 2:
	gc.angle.set(0);
	g.first.push_back(gc);

	GeneLocalAttribute ga;
	// body size (sq meters)
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.location.set(0);
	ga.value.set(0.01f);
	g.first.push_back(ga);

	// first bone size:
	ga.location.set(0b1001);
	ga.value.set(8);
	g.first.push_back(ga);

	// first bone aspect
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	g.first.push_back(ga);

	// second bone size:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.location.set(0b1001001001);
	ga.value.set(8);
	g.first.push_back(ga);

	// second bone aspect
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	g.first.push_back(ga);

	// first muscle size

	g.second = g.first; // make a duplicate of all genes into the second chromosome

	return new Bug(g, 0.08f);
}
