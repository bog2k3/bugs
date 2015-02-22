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
#include "../neuralnet/functions.h"
#include "../math/math2D.h"
#include "../body-parts/ZygoteShell.h"
#include "../body-parts/Torso.h"
#include "../body-parts/BodyConst.h"
#include "../body-parts/EggLayer.h"
#include "../utils/log.h"
#include "../World.h"
#include "Bug/ISensor.h"
#include "Bug/Motor.h"
#include "Gamete.h"

const float DECODE_FREQUENCY = 5.f; // genes per second
const float DECODE_PERIOD = 1.f / DECODE_FREQUENCY; // seconds

Bug::Bug(Genome const &genome, float zygoteMass, glm::vec2 position)
	: genome_(genome)
	, neuralNet_(new NeuralNet())
	, ribosome_(nullptr)
	, isAlive_(true)
	, isDeveloping_(true)
	, tRibosomeStep_(0)
	, body_(nullptr)
	, zygoteShell_(nullptr)
	, growthMassBuffer_(0)
	, maxGrowthMassBuffer_(0)
	, cachedLeanMass_(0)
	, cachedMassDirty_(false)
	, initialFatMassRatio_(BodyConst::initialFatMassRatio)
	, minFatMasRatio_(BodyConst::initialMinFatMassRatio)
	, adultLeanMass_(BodyConst::initialAdultLeanMass)
	, growthSpeed_(BodyConst::initialGrowthSpeed)
	, reproductiveMassRatio_(BodyConst::initialReproductiveMassRatio)
	, eggMass_(BodyConst::initialEggMass)
{
	// create embryo shell:
	zygoteShell_ = new ZygoteShell(position, zygoteMass);
	// zygote mass determines the overall bug size after decoding -> must have equal overal mass
	zygoteShell_->setUpdateList(bodyPartsUpdateList_);

	body_ = new Torso();
	zygoteShell_->add(body_, 0);
	body_->onFoodProcessed.add(std::bind(&Bug::onFoodProcessed, this, std::placeholders::_1));
	body_->onMotorLinesDetached.add(std::bind(&Bug::onMotorLinesDetached, this, std::placeholders::_1));
	body_->onBodyMassChanged.add([this] { cachedMassDirty_ = true; });
	body_->setGenome(&genome_);
	ribosome_ = new Ribosome(this);

	mapBodyAttributes_[GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO] = &initialFatMassRatio_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO] = &minFatMasRatio_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_ADULT_LEAN_MASS] = &adultLeanMass_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_GROWTH_SPEED] = &growthSpeed_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_REPRODUCTIVE_MASS_RATIO] = &reproductiveMassRatio_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_EGG_MASS] = &eggMass_;

	sensors_.push_back(&lifeTimeSensor_);
}

Bug::~Bug() {
	if (zygoteShell_) {
		zygoteShell_->destroy();
		zygoteShell_ = nullptr;
	}
	else if (body_) {
		body_->destroy();
		body_ = nullptr;
	}
	if (ribosome_)
		delete ribosome_;
}

void Bug::updateEmbryonicDevelopment(float dt) {
	// developing embryo
	tRibosomeStep_ += dt;
	if (tRibosomeStep_ >= DECODE_PERIOD) {
		tRibosomeStep_ -= DECODE_PERIOD;
		isDeveloping_ = ribosome_->step();
		if (!isDeveloping_) {	// finished development?
			if (!isAlive_) {
				// embryo not viable, discarded.
				LOGLN("Embryo not viable. DISCARDED.");
				zygoteShell_->die_tree();
				body_->detach(false);
				body_->destroy();
				body_ = nullptr;
				return;
			}

			float currentMass = body_->getMass_tree();
			float zygMass = zygoteShell_->getMass();

			fixAllGeneValues();

			// compute fat amount and scale up the torso to the correct size
			float fatMass = zygMass * initialFatMassRatio_;
			body_->setInitialFatMass(fatMass);
			cachedLeanMass_ = zygMass - fatMass;

			zygoteShell_->updateCachedDynamicPropsFromBody();
			// commit all changes and create the physics bodys and fixtures:
			body_->commit_tree(cachedLeanMass_/currentMass);

			// delete embryo shell
			body_->detach(false);
			zygoteShell_->destroy();
			zygoteShell_ = nullptr;

			delete ribosome_;
			ribosome_ = nullptr;
		}
	}
}

void Bug::fixAllGeneValues() {
	initialFatMassRatio_.reset(clamp(initialFatMassRatio_.get(), 0.f, 1.f));
	minFatMasRatio_.reset(clamp(minFatMasRatio_.get(), 0.f, 1.f));
	adultLeanMass_.reset(clamp(adultLeanMass_.get(), 0.f, 1.e+20f));
	growthSpeed_.reset(clamp(growthSpeed_.get(), 0.f, 1.e+20f));
	reproductiveMassRatio_.reset(clamp(reproductiveMassRatio_.get(), 0.f, 1.f));
	eggMass_.reset(clamp(eggMass_.get(), 0.f, 1.e+20f));
	maxGrowthMassBuffer_ = growthSpeed_ * 100;	// can hold enough growth mass for 100 seconds
}

void Bug::updateDeadDecaying(float dt) {
	// dead, decaying
	// body parts loose their nutrient value gradually until they are deleted
}

void Bug::update(float dt) {
	if (!isAlive_) {
		updateDeadDecaying(dt);
		return;
	}
	if (isDeveloping_) {
		updateEmbryonicDevelopment(dt);
		return;
	}

	lifeTimeSensor_.update(dt);
	bodyPartsUpdateList_.update(dt);
	neuralNet_->iterate();

	if (body_->getFatMass() <= 0 && body_->getBufferedEnergy() <= 0) {
		// we just depleted our energy supply and died
		isAlive_ = false;
		body_->die_tree();
		return;
	}

	//LOGLN("leanMass: "<<body_->getMass_tree()<<"  eggMassBuf: "<<eggMassBuffer_<<";  fatMass: "<<body_->getFatMass()<<";  energy: "<<body_->getBufferedEnergy());

	if (cachedMassDirty_) {
		// some part broke up, must recompute some things
		float oldLeanMass = cachedLeanMass_;
		body_->resetCachedMass();
#warning "getMass_tree() includes fat too after purging initialization data"
		cachedLeanMass_ = body_->getMass_tree();
		adultLeanMass_.changeRel(cachedLeanMass_ / oldLeanMass);
		cachedMassDirty_ = false;
	}
	if (cachedLeanMass_ < adultLeanMass_) {
		// juvenile, growing
		// max growth speed is dictated by genes
		float massToGrow = growthSpeed_ * dt;
		if (massToGrow > growthMassBuffer_)
			massToGrow = growthMassBuffer_;
		growthMassBuffer_ -= massToGrow;
		cachedLeanMass_ += massToGrow;
		body_->applyScale_tree(cachedLeanMass_ / body_->getMass_tree());
	} else {
		// adult life
	}
}

void Bug::draw(RenderContext const &ctx) {
	if (zygoteShell_)
		zygoteShell_->draw_tree(ctx);
	else
		body_->draw_tree(ctx);
}

void Bug::onFoodProcessed(float mass) {
	/*
	 * if fat is below critical ratio, all food goes to replenish it
	 * else if not at full size, a fraction of food is used for filling up the growth buffer.
	 * the rest of the food turns into energy and fat.
	 */
	//LOGLN("PROCESS_FOOD "<<mass<<"======================");
	if (cachedMassDirty_)
		return;
	float fatMassRatio = body_->getFatMass() / (cachedLeanMass_ + body_->getFatMass());
	float growthMass = 0;
	float eggMass = 0;
	if (fatMassRatio >= minFatMasRatio_) {
		// use some food to make eggs:
		eggMass = mass * reproductiveMassRatio_;
		float totalFoodRequired = 0;
		for (int i=0; i<(int)eggLayers_.size(); i++)
			totalFoodRequired += eggLayers_[i]->getFoodRequired();
		if (totalFoodRequired > 0) {
			float availPerTotal = eggMass / totalFoodRequired;
			for (int i=0; i<(int)eggLayers_.size(); i++)
				eggLayers_[i]->useFood(eggLayers_[i]->getFoodRequired() * availPerTotal);
		} else
			eggMass = 0;
		// use some food to grow the body:
		if (cachedLeanMass_ < adultLeanMass_) {
			growthMass = mass - eggMass;
			float transferedMass = maxGrowthMassBuffer_ - growthMassBuffer_;
			if (growthMass < transferedMass)
				transferedMass = growthMass;
			else
				growthMass = transferedMass;
			growthMassBuffer_ += transferedMass;
		}
	}
	// use the left food to make fat and energy:
	body_->replenishEnergyFromMass(mass - eggMass - growthMass);
}

void Bug::onMotorLinesDetached(std::vector<int> const& lines) {
	if (!isAlive_)
		return;
	for (int i : lines) {
		assert(i < neuralNet_->outputs.size() && neuralNet_->outputs[i]->pParentNeuron);
		neuralNet_->outputs[i]->pParentNeuron->output.getTargets().clear();
	}
}

Chromosome Bug::createBasicChromosome() {
	Chromosome c;

	// body size (sq meters)
	GeneAttribute ga;
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.1f * 0.1f);
	c.push_back(ga);

	//body attributes
	GeneBodyAttribute gba;
	gba.attribute = GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO;
	gba.value.set(0.5f);
	c.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO;
	gba.value.set(0.1f);
	c.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_ADULT_LEAN_MASS;
	gba.value.set(4.f);
	c.push_back(gba);

	// neural system

	// neuron #0 transfer:
	GeneTransferFunction gt;
	gt.targetNeuron.set(0);
	gt.functionID.set(FN_CONSTANT);
	c.push_back(gt);
	// neuron #0 constant:
	GeneNeuralConstant gnc;
	gnc.targetNeuron.set(0);
	gnc.value.set(0.5f);
	c.push_back(gnc);

	// neuron #1 transfer:
	gt.targetNeuron.set(1);
	gt.functionID.set(FN_SIN);
	c.push_back(gt);

	// neuron #2 transfer:
	gt.targetNeuron.set(2);
	gt.functionID.set(FN_SIN);
	c.push_back(gt);

	// neuron #3 transfer:
	gt.targetNeuron.set(3);
	gt.functionID.set(FN_SIN);
	c.push_back(gt);

	// neuron #4 transfer:
	gt.targetNeuron.set(4);
	gt.functionID.set(FN_SIN);
	c.push_back(gt);

	// neuron #5 transfer:
	gt.targetNeuron.set(5);
	gt.functionID.set(FN_CONSTANT);
	c.push_back(gt);
	// neuron #5 constant:
	gnc.targetNeuron.set(5);
	gnc.value.set(0.4f);
	c.push_back(gnc);

	// neuron #6 transfer:
	gt.targetNeuron.set(6);
	gt.functionID.set(FN_ONE);
	c.push_back(gt);

	// neuron #7 transfer:
	gt.targetNeuron.set(7);
	gt.functionID.set(FN_THRESHOLD);
	c.push_back(gt);
	// neuron #7 threshold value:
	gnc.targetNeuron.set(7);
	gnc.value.set(0);
	c.push_back(gnc);

	// neuron #8 transfer:
	gt.targetNeuron.set(8);
	gt.functionID.set(FN_THRESHOLD);
	c.push_back(gt);
	// neuron #8 threshold value:
	gnc.targetNeuron.set(8);
	gnc.value.set(0);
	c.push_back(gnc);

	const float musclePeriod = 2.f; // seconds

	GeneSynapse gs;

	// synapse 0 to 1
	gs.from.set(0);
	gs.to.set(1);
	gs.weight.set(2*PI/musclePeriod);
	c.push_back(gs);

	// synapse i[-1] (time) to 1
	gs.from.set(-1);
	gs.to.set(1);
	gs.weight.set(2*PI/musclePeriod);
	c.push_back(gs);

	// synapse i[-1] (time) to 2
	gs.from.set(-1);
	gs.to.set(2);
	gs.weight.set(2*PI/musclePeriod);
	c.push_back(gs);

	// synapse 1 to 3
	gs.from.set(1);
	gs.to.set(3);
	gs.weight.set(1.f);
	c.push_back(gs);

	// synapse 2 to 4
	gs.from.set(2);
	gs.to.set(4);
	gs.weight.set(1.f);
	c.push_back(gs);

	// synapse 5 to 6
	gs.from.set(5);
	gs.to.set(6);
	gs.weight.set(1.f);
	c.push_back(gs);

	// synapse 4 to 7
	gs.from.set(4);
	gs.to.set(7);
	gs.weight.set(1.f);
	c.push_back(gs);

	// synapse 4 to 8
	gs.from.set(4);
	gs.to.set(8);
	gs.weight.set(-1.f);
	c.push_back(gs);

	// synapse 7 to o[-3] (muscle 1)
	gs.from.set(7);
	gs.to.set(-3);
	gs.weight.set(1.f);
	c.push_back(gs);

	// synapse 8 to o[-4] (muscle 2)
	gs.from.set(8);
	gs.to.set(-4);
	gs.weight.set(1.f);
	c.push_back(gs);

	// synapse 3 to 6
	gs.from.set(3);
	gs.to.set(6);
	gs.weight.set(-0.7f);
	c.push_back(gs);

	// synapse 6 to o[-9] (gripper)
	gs.from.set(6);
	gs.to.set(-9);
	gs.weight.set(1.f);
	c.push_back(gs);

	// Mouth:
	GeneCommand gc;
	gc.command = GENE_DEV_GROW;
	gc.angle.set(0);
	gc.part_type = GENE_PART_MOUTH;
	gc.genomeOffset.set(4); // stop
	gc.age = 10;
	c.push_back(gc);

	// Egg-layer:
	gc.angle.set(3*PI/4);
	gc.part_type = GENE_PART_EGGLAYER;
	gc.genomeOffset.set(3); // stop
	gc.age = 9;
	c.push_back(gc);

	// bone 1:
	gc.age = 8;
	gc.angle.set(PI);
	gc.part_type = GENE_PART_BONE;
	gc.genomeOffset.set(4);
	gc.genomeOffsetJoint.set(2);
	gc.genomeOffsetMuscle1.set(11);
	gc.genomeOffsetMuscle2.set(11);
	c.push_back(gc);

	c.push_back(GeneStop());
	c.push_back(GeneStop());
	c.push_back(GeneStop());

	// bone 2:
	gc.age = 7;
	gc.angle.set(0);
	gc.genomeOffset.set(5);
	gc.genomeOffsetJoint.set(4);
	gc.genomeOffsetMuscle1.set(4);
	gc.genomeOffsetMuscle2.set(4);
	c.push_back(gc);

	// bone 1 size:
	ga.value.set(0.08f * 0.01f);
	c.push_back(ga);

	// bone 1 aspect
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	c.push_back(ga);

	// bone 1 rotation
	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(PI/8);
	c.push_back(ga);

	c.push_back(GeneStop());

	// the gripper:
	gc.age = 6;
	gc.part_type = GENE_PART_GRIPPER;
	gc.genomeOffsetJoint.set(3);
	gc.genomeOffsetMuscle1.set(3);
	gc.genomeOffsetMuscle2.set(3);
	c.push_back(gc);

	// bone 2 size:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.push_back(ga);

	// bone 2 aspect
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	c.push_back(ga);

	c.push_back(GeneStop());

	// muscle 1 & 2 size
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(1.e-3f);
	c.push_back(ga);

	c.push_back(GeneStop());

	return c;
}

Bug* Bug::newBasicBug(glm::vec2 position) {
	Genome g;
	g.first = g.second = createBasicChromosome(); // make a duplicate of all genes into the second chromosome
	return new Bug(g, 2*BodyConst::initialEggMass, position);
}

Bug* Bug::newBasicMutantBug(glm::vec2 position) {
	Genome g;
	g.first = g.second = createBasicChromosome();
	GeneticOperations::alterChromosome(g.first);
	GeneticOperations::alterChromosome(g.second);
	return new Bug(g, 2*BodyConst::initialEggMass, position);
}
