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
#include "../objects/body-parts/ZygoteShell.h"
#include "../objects/body-parts/Torso.h"

const float DECODE_FREQUENCY = 1.f; // genes per second
const float DECODE_PERIOD = 1.f / DECODE_FREQUENCY; // seconds
const float ZYGOTE_ENERGY_DENSITY = 1000; // Joules per m^2

Bug::Bug(Genome const &genome, float zygoteSize, glm::vec2 position)
	: genome_(genome)
	, neuralNet_(new NeuralNet())
	, ribosome_(nullptr)
	, isAlive_(true)
	, isDeveloping_(true)
	, tRibosomeStep_(0)
	, energy_(zygoteSize * ZYGOTE_ENERGY_DENSITY)
	, scale_(0.05f)
	, scaledEnergy_(0)
	, body_(nullptr)
	, zygoteShell_(nullptr)
{
	// pe masura ce se dezvolta, fiecare noua parte consuma energie.
	// energia disponibila in zigot si energia necesara dezvoltarii determina scala initiala dupa decodare.

	// create embryo shell:
	zygoteShell_ = new ZygoteShell(zygoteSize, PhysicsProperties(position, 0));
	body_ = new Torso(zygoteShell_, PhysicsProperties(glm::vec2(0), 0));
	ribosome_ = new Ribosome(this);
}

Bug::~Bug() {
}

template<> void update(Bug* b, float dt) {
	b->update(dt);
}

void Bug::update(float dt) {
	if (isAlive_) {
		if (isDeveloping_) {
			// developing embryo
			tRibosomeStep_ += dt;
			if (tRibosomeStep_ >= DECODE_PERIOD) {
				tRibosomeStep_ -= DECODE_PERIOD;
				isDeveloping_ = ribosome_->step();
				if (!isDeveloping_) {
					// delete embryo shell
					body_->changeParent(nullptr);
					delete zygoteShell_;
					zygoteShell_ = nullptr;

					// commit all changes and create the physics bodys and fixtures:
					body_->commit_tree();
				}
			}
		} else {
			if (scale_ < 1) {
				// juvenile, growing
				// growth happens by scaling up size and scaling down energy proportionally;
				// growth speed is dictated by genes
			} else {
				// adult life
				// unused energy is stored by growing the torso
			}
		}
	} else {
		// dead, decaying
		// body parts loose their nutrient value gradually until they are deleted
	}
}

Bug* Bug::newBasicBug(glm::vec2 position) {
	Genome g;

	GeneCommand gc;
	// first bone:
	gc.command = GENE_DEV_GROW;
	gc.angle.set(PI);
	gc.part_type = GENE_PART_BONE;
	gc.location.set(0);
	g.first.push_back(gc);

	// second bone:
	gc.location.set(0b10001);
	gc.angle.set(0);
	g.first.push_back(gc);

	// the gripper:
	gc.location.set(0b1000110001);
	gc.part_type = GENE_PART_GRIPPER;
	//g.first.push_back(gc);

	// bone 1 muscle 1:
	gc.location.set(0b10001);
	gc.angle.set(PI/2);
	gc.part_type = GENE_PART_MUSCLE;
	//g.first.push_back(gc);

	// bone 1 muscle 2:
	gc.angle.set(-PI/2);
	//g.first.push_back(gc);

	GeneLocalAttribute ga;
	// body size (sq meters)
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.location.set(0);
	ga.value.set(0.1f * 0.1f);
	//g.first.push_back(ga);

	// first bone size:
	ga.location.set(0b10001);
	ga.value.set(0.08f * 0.01f);
	//g.first.push_back(ga);

	// first bone aspect
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	//g.first.push_back(ga);

	// second bone size:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.location.set(0b1000110001);
	ga.value.set(0.08f * 0.01f);
	//g.first.push_back(ga);

	// second bone aspect
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	//g.first.push_back(ga);

	// first muscle size

	g.second = g.first; // make a duplicate of all genes into the second chromosome

	return new Bug(g, 0.08f, position);
}

