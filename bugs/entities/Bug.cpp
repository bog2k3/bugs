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
	// create embryo shell:
	zygoteShell_ = new ZygoteShell(zygoteSize);		// zygote mass determines the overall bug size after decoding -> must have equal overal mass
	body_ = new Torso(zygoteShell_);
	body_->setUpdateList(bodyPartsUpdateList_);
	ribosome_ = new Ribosome(this);
}

Bug::~Bug() {
}

template<> void update(Bug*& b, float dt) {
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
					zygoteShell_->updateCachedDynamicPropsFromBody();
					// commit all changes and create the physics bodys and fixtures:
					body_->commit_tree();

					// delete embryo shell
					body_->changeParent(nullptr);
					delete zygoteShell_;
					zygoteShell_ = nullptr;
				}
			}
		} else {
			bodyPartsUpdateList_.update(dt);
			if (scale_ < 1) {
				// juvenile, growing
				// growth happens by scaling up size and scaling down energy proportionally;
				// growth speed is dictated by genes

				if (scale_ >= 1) {
					// finished developing, discard all initialization data which is not useful any more:
					body_->purge_initializationData_tree();
				}
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

	// first bone:
	GeneLocation gl;
	gl.location[0].set(1 << 15);
	g.first.push_back(gl);
	GeneCommand gc;
	gc.command = GENE_DEV_GROW;
	gc.angle.set(PI);
	gc.part_type = GENE_PART_BONE;
	g.first.push_back(gc);

	// second bone:
	gl.location[0].set(1);
	gl.location[1].set(1);
	gl.location[2].set(1 << 15);
	g.first.push_back(gl);
	gc.angle.set(0);
	g.first.push_back(gc);

	// the gripper:
	gl.location[2].set(1);
	gl.location[3].set(1);
	gl.location[4].set(1 << 15);
	g.first.push_back(gl);
	gc.part_type = GENE_PART_GRIPPER;
	g.first.push_back(gc);

	// body size (sq meters)
	GeneLocalAttribute ga;
	gl.location[0].set(1 << 15);
	g.first.push_back(gl);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.1f * 0.1f);
	g.first.push_back(ga);

	// first bone size:
	gl.location[0].set(1);
	gl.location[2].set(1 << 15);
	g.first.push_back(gl);
	ga.value.set(0.08f * 0.01f);
	g.first.push_back(ga);

	// first bone aspect
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	g.first.push_back(ga);

	// second bone size:
	gl.location[2].set(1);
	g.first.push_back(gl);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	g.first.push_back(ga);

	// second bone aspect
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	g.first.push_back(ga);

	// first muscle size
	// ...

	g.second = g.first; // make a duplicate of all genes into the second chromosome

	return new Bug(g, 0.08f, position);
}

