/*
 * Bug.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#include "Bug.h"
#include "IMotor.h"
#include "ISensor.h"
#include "../neuralnet/Network.h"
#include "../genetics/Gene.h"
#include "../genetics/GeneDefinitions.h"
#include "../genetics/constants.h"
#include "../genetics/Ribosome.h"
#include "../neuralnet/functions.h"
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
	, lifeTime_(0)
	, initialFatMassRatio_(BodyConst::initialFatMassRatio)
	, minFatMasRatio_(BodyConst::initialMinFatMassRatio)
	, adultLeanMass_(BodyConst::initialAdultLeanMass)
{
	// create embryo shell:
	zygoteShell_ = new ZygoteShell(position, zygoteSize);
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

			lifeTime_ += dt;
			/*const float musclePeriod = 2.f; // seconds
			float sval = sinf(sinf(2*PI/musclePeriod * lifeTime_));
			float mval1 = sval > 0 ? sval : 0;
			float mval2 = sval < 0 ? -sval : 0;
			motors_[0]->action(mval1);
			motors_[1]->action(mval2);
			float svalg = sinf(sinf(2*PI/musclePeriod * (lifeTime_+0.5f)));
			motors_[6]->action((-svalg + 0.7f)*0.7f);	// the gripper*/


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

	// body shape

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
	gl.location[0].set(2);
	gl.location[1].set(1 << 15);
	g.first.push_back(gl);
	gla.attribute = GENE_ATTRIB_SIZE;
	gla.value.set(0.5e-3f);
	g.first.push_back(gla);

	// second muscle size
	gl.location[0].set(4);
	g.first.push_back(gl);
	g.first.push_back(gla);


	//body attributes

	GeneBodyAttribute gba;
	gba.attribute = GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO;
	gba.value.set(0.5f);
	g.first.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO;
	gba.value.set(0.1f);
	g.first.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_ADULT_LEAN_MASS;
	gba.value.set(4.f);
	g.first.push_back(gba);


	// neural system

	// neuron #0 transfer:
	GeneTransferFunction gt;
	gt.targetNeuron.set(0);
	gt.functionID.set(FN_CONSTANT);
	g.first.push_back(gt);
	// neuron #0 constant:
	GeneNeuralConstant gnc;
	gnc.targetNeuron.set(0);
	gnc.value.set(0.5f);
	g.first.push_back(gnc);

	// neuron #1 transfer:
	gt.targetNeuron.set(1);
	gt.functionID.set(FN_SIN);
	g.first.push_back(gt);

	// neuron #2 transfer:
	gt.targetNeuron.set(2);
	gt.functionID.set(FN_SIN);
	g.first.push_back(gt);

	// neuron #3 transfer:
	gt.targetNeuron.set(3);
	gt.functionID.set(FN_SIN);
	g.first.push_back(gt);

	// neuron #4 transfer:
	gt.targetNeuron.set(4);
	gt.functionID.set(FN_SIN);
	g.first.push_back(gt);

	// neuron #5 transfer:
	gt.targetNeuron.set(5);
	gt.functionID.set(FN_CONSTANT);
	g.first.push_back(gt);
	// neuron #5 constant:
	gnc.targetNeuron.set(5);
	gnc.value.set(0.49f);
	g.first.push_back(gnc);

	// neuron #6 transfer:
	gt.targetNeuron.set(6);
	gt.functionID.set(FN_ONE);
	g.first.push_back(gt);

	// neuron #7 transfer:
	gt.targetNeuron.set(7);
	gt.functionID.set(FN_THRESHOLD);
	g.first.push_back(gt);
	// neuron #7 threshold value:
	gnc.targetNeuron.set(7);
	gnc.value.set(0);
	g.first.push_back(gnc);

	// neuron #8 transfer:
	gt.targetNeuron.set(8);
	gt.functionID.set(FN_THRESHOLD);
	g.first.push_back(gt);
	// neuron #8 threshold value:
	gnc.targetNeuron.set(8);
	gnc.value.set(0);
	g.first.push_back(gnc);

	const float musclePeriod = 2.f; // seconds

	// synapse i[-1] (time) to 1
	GeneSynapse gs;
	gs.from.set(-1);
	gs.to.set(1);
	gs.weight.set(2*PI/musclePeriod);
	g.first.push_back(gs);

	// synapse i[-1] (time) to 2
	gs.from.set(-1);
	gs.to.set(2);
	gs.weight.set(2*PI/musclePeriod);
	g.first.push_back(gs);

	// synapse 0 to 1
	gs.from.set(0);
	gs.to.set(1);
	gs.weight.set(2*PI/musclePeriod);
	g.first.push_back(gs);

	// synapse 2 to 4
	gs.from.set(2);
	gs.to.set(4);
	gs.weight.set(1.f);
	g.first.push_back(gs);

	// synapse 4 to 7
	gs.from.set(4);
	gs.to.set(7);
	gs.weight.set(1.f);
	g.first.push_back(gs);

	// synapse 4 to 8
	gs.from.set(4);
	gs.to.set(8);
	gs.weight.set(-1.f);
	g.first.push_back(gs);

	// synapse 7 to o[-1] (muscle 1)
	gs.from.set(7);
	gs.to.set(-1);
	gs.weight.set(1.f);
	g.first.push_back(gs);

	// synapse 8 to o[-2] (muscle 2)
	gs.from.set(8);
	gs.to.set(-2);
	gs.weight.set(1.f);
	g.first.push_back(gs);

	// synapse 1 to 3
	gs.from.set(1);
	gs.to.set(3);
	gs.weight.set(1.f);
	g.first.push_back(gs);

	// synapse 3 to 6
	gs.from.set(3);
	gs.to.set(6);
	gs.weight.set(-0.7f);
	g.first.push_back(gs);

	// synapse 5 to 6
	gs.from.set(5);
	gs.to.set(6);
	gs.weight.set(1.f);
	g.first.push_back(gs);

	// synapse 6 to o[-3] (gripper)
	gs.from.set(6);
	gs.to.set(-3);
	gs.weight.set(1.f);
	g.first.push_back(gs);

	g.second = g.first; // make a duplicate of all genes into the second chromosome

	return new Bug(g, 0.08f, position);
}

