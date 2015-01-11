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
#include "../objects/body-parts/BodyConst.h"

const float DECODE_FREQUENCY = 5.f; // genes per second
const float DECODE_PERIOD = 1.f / DECODE_FREQUENCY; // seconds

Bug::Bug(Genome const &genome, float zygoteSize, glm::vec2 position)
	: genome_(genome)
	, neuralNet_(new NeuralNet())
	, ribosome_(nullptr)
	, isAlive_(true)
	, isDeveloping_(true)
	, tRibosomeStep_(0)
	, body_(nullptr)
	, zygoteShell_(nullptr)
	, initialFatMassRatio_(BodyConst::initialFatMassRatio)
	, minFatMasRatio_(BodyConst::initialMinFatMassRatio)
	, adultLeanMass_(BodyConst::initialAdultLeanMass)
{
	// create embryo shell:
	zygoteShell_ = new ZygoteShell(zygoteSize);
	// zygote mass determines the overall bug size after decoding -> must have equal overal mass

	body_ = new Torso(zygoteShell_);
	body_->setUpdateList(bodyPartsUpdateList_);
	ribosome_ = new Ribosome(this);

	mapBodyAttributes_[GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO] = &initialFatMassRatio_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO] = &minFatMasRatio_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_ADULT_LEAN_MASS] = &adultLeanMass_;
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
					float currentMass = body_->getMass_tree();
					float zygMass = zygoteShell_->getMass();

					// compute fat amount and scale up the torso to the correct size
					float fatMass = zygMass * initialFatMassRatio_ / (initialFatMassRatio_+1);
					body_->setFatMass(fatMass);
					body_->applyScale_tree((zygMass-fatMass)/currentMass);

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
			/*static float crtScale = 1.f;
			if (crtScale < 10) {
				crtScale += 0.001f * dt;
				body_->applyScale_tree(crtScale);
			}*/
			bodyPartsUpdateList_.update(dt);
			if (true /* not adult scale yet*/) {
				// juvenile, growing
				// growth happens by scaling up size and scaling down energy proportionally;
				// growth speed is dictated by genes

				//body_->applyScale_tree(1.01f);

				if (false /* reached adulthood scale?*/) {
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

	gl.location[0].set(1);
	gl.location[1].set(1);
	gl.location[2].set(1<<15);
	g.first.push_back(gl);
	GeneLocalAttribute gla;
	gla.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	gla.value.set(PI/8);
	g.first.push_back(gla);

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
	gl.location[0].set(1 << 15);
	g.first.push_back(gl);
	gla.attribute = GENE_ATTRIB_SIZE;
	gla.value.set(0.1f * 0.1f);
	g.first.push_back(gla);

	// first bone size:
	gl.location[0].set(1);
	gl.location[2].set(1 << 15);
	g.first.push_back(gl);
	gla.value.set(0.08f * 0.01f);
	g.first.push_back(gla);

	// first bone aspect
	gla.attribute = GENE_ATTRIB_ASPECT_RATIO;
	gla.value.set(4);
	g.first.push_back(gla);

	// second bone size:
	gl.location[2].set(1);
	g.first.push_back(gl);
	gla.attribute = GENE_ATTRIB_SIZE;
	gla.value.set(0.08f * 0.01f);
	g.first.push_back(gla);

	// second bone aspect
	gla.attribute = GENE_ATTRIB_ASPECT_RATIO;
	gla.value.set(4);
	g.first.push_back(gla);

	// first muscle size
	// ...

	g.second = g.first; // make a duplicate of all genes into the second chromosome

	return new Bug(g, 0.08f, position);
}

