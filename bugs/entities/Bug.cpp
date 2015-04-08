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
#include "../body-parts/BodyPart.h"
#include "../body-parts/Joint.h"
#include "../utils/log.h"
#include "../World.h"
#include "../serialization/BinaryStream.h"
#include "../serialization/GenomeSerialization.h"
#include "Bug/ISensor.h"
#include "Bug/Motor.h"
#include "Gamete.h"

const float DECODE_FREQUENCY = 5.f; // genes per second
const float DECODE_PERIOD = 1.f / DECODE_FREQUENCY; // seconds

Bug::Bug(Genome const &genome, float zygoteMass, glm::vec2 position, glm::vec2 velocity)
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
	zygoteShell_ = new ZygoteShell(position, velocity, zygoteMass);
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
	} else {
		for (auto bp : deadBodyParts_)
			if (bp != nullptr)
				bp->destroy();
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

			body_->applyRecursive([this](BodyPart* part) {
				part->onDied.add([this](BodyPart *dying) {
					deadBodyParts_.push_back(dying);
					dying->removeAllLinks();
				});
				return false;
			});
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
	// body parts loose their nutrient value gradually until they are deleted
	for (unsigned i=0; i<deadBodyParts_.size(); i++) {
		if (!!!deadBodyParts_[i])
			continue;
		deadBodyParts_[i]->consumeFoodValue(dt * WorldConst::BodyDecaySpeed);
		if (deadBodyParts_[i]->getFoodValue() <= 0) {
			deadBodyParts_[i]->destroy();
			bodyPartsUpdateList_.remove(deadBodyParts_[i]);
			deadBodyParts_[i] = nullptr;
		}
	}
}

void Bug::kill() {
	LOGLN("bug DIED");
	isAlive_ = false;
	body_->die_tree();
	body_ = nullptr;
}

void Bug::update(float dt) {
	if (isDeveloping_) {
		updateEmbryonicDevelopment(dt);
		return;
	}

	lifeTimeSensor_.update(dt);
	bodyPartsUpdateList_.update(dt);
	updateDeadDecaying(dt); // this updates all dead body parts

	if (!isAlive_)
		return;

	neuralNet_->iterate();

	if (body_->getFatMass() <= 0 && body_->getBufferedEnergy() <= 0) {
		// we just depleted our energy supply and died
		kill();
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
	else if (body_)
		body_->draw_tree(ctx);
	for (auto bp : deadBodyParts_)
		if (bp)
			bp->draw(ctx);
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
		assertDbg(i < neuralNet_->outputs.size() && neuralNet_->outputs[i]->pParentNeuron);
		neuralNet_->outputs[i]->pParentNeuron->output.getTargets().clear();
	}
}

Chromosome Bug::createBasicChromosome() {
	Chromosome c;

	// body size (sq meters)
	GeneAttribute ga;
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.1f * 0.1f);
	c.genes.push_back(ga);

	//body attributes
	GeneBodyAttribute gba;
	gba.attribute = GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO;
	gba.value.set(0.5f);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO;
	gba.value.set(0.1f);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_ADULT_LEAN_MASS;
	gba.value.set(4.f);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_GROWTH_SPEED;
	gba.value.set(BodyConst::initialGrowthSpeed);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_EGG_MASS;
	gba.value.set(BodyConst::initialEggMass);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_REPRODUCTIVE_MASS_RATIO;
	gba.value.set(BodyConst::initialReproductiveMassRatio);
	c.genes.push_back(gba);

	// neural system

	// neuron #0 transfer:
	GeneTransferFunction gt;
	gt.targetNeuron.set(0);
	gt.functionID.set((int)transferFuncNames::FN_CONSTANT);
	c.genes.push_back(gt);
	// neuron #0 constant:
	GeneNeuralConstant gnc;
	gnc.targetNeuron.set(0);
	gnc.value.set(1.7f);
	c.genes.push_back(gnc);

	// neuron #1 transfer:
	gt.targetNeuron.set(1);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);

	// neuron #2 transfer:
	gt.targetNeuron.set(2);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);

	// neuron #3 transfer:
	gt.targetNeuron.set(3);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);

	// neuron #4 transfer:
	gt.targetNeuron.set(4);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);

	// neuron #5 transfer:
	gt.targetNeuron.set(5);
	gt.functionID.set((int)transferFuncNames::FN_CONSTANT);
	c.genes.push_back(gt);
	// neuron #5 constant:
	gnc.targetNeuron.set(5);
	gnc.value.set(0.1f);
	c.genes.push_back(gnc);

	// neuron #6 transfer:
	gt.targetNeuron.set(6);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);

	// neuron #7 transfer:
	gt.targetNeuron.set(7);
	gt.functionID.set((int)transferFuncNames::FN_THRESHOLD);
	c.genes.push_back(gt);
	// neuron #7 threshold value:
	gnc.targetNeuron.set(7);
	gnc.value.set(0);
	c.genes.push_back(gnc);

	// neuron #8 transfer:
	gt.targetNeuron.set(8);
	gt.functionID.set((int)transferFuncNames::FN_THRESHOLD);
	c.genes.push_back(gt);
	// neuron #8 threshold value:
	gnc.targetNeuron.set(8);
	gnc.value.set(0);
	c.genes.push_back(gnc);

	const float musclePeriod = 3.f; // seconds

	GeneSynapse gs;

	// synapse 0 to 1
	gs.from.set(0);
	gs.to.set(1);
	gs.weight.set(2*PI/musclePeriod);
	c.genes.push_back(gs);

	// synapse i[-1] (time) to 1
	gs.from.set(-1);
	gs.to.set(1);
	gs.weight.set(2*PI/musclePeriod);
	c.genes.push_back(gs);

	// synapse i[-1] (time) to 2
	gs.from.set(-1);
	gs.to.set(2);
	gs.weight.set(2*PI/musclePeriod);
	c.genes.push_back(gs);

	// synapse 1 to 3
	gs.from.set(1);
	gs.to.set(3);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 2 to 4
	gs.from.set(2);
	gs.to.set(4);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 5 to 6
	gs.from.set(5);
	gs.to.set(6);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 4 to 7
	gs.from.set(4);
	gs.to.set(7);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 4 to 8
	gs.from.set(4);
	gs.to.set(8);
	gs.weight.set(-1.f);
	c.genes.push_back(gs);

	// synapse 7 to o[-3] (muscle 1)
	gs.from.set(7);
	gs.to.set(-3);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 8 to o[-4] (muscle 2)
	gs.from.set(8);
	gs.to.set(-4);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 3 to 6
	gs.from.set(3);
	gs.to.set(6);
	gs.weight.set(-0.7f);
	c.genes.push_back(gs);

	// synapse 6 to o[-9] (gripper)
	gs.from.set(6);
	gs.to.set(-9);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// Mouth:
	GeneCommand gc;
	gc.command = GENE_DEV_GROW;
	gc.angle.set(0);
	gc.part_type = GENE_PART_MOUTH;
	gc.genomeOffset.set(5);
	gc.age = 10;
	c.genes.push_back(gc);

	// Egg-layer:
	gc.angle.set(2.5f*PI/4);
	gc.part_type = GENE_PART_EGGLAYER;
	gc.genomeOffset.set(10);
	gc.age = 9;
	c.genes.push_back(gc);

	// bone 1:
	gc.age = 8;
	gc.angle.set(PI);
	gc.part_type = GENE_PART_BONE;
	gc.genomeOffset.set(21);
	gc.genomeOffsetJoint.set(15);
	gc.genomeOffsetMuscle1.set(65);
	gc.genomeOffsetMuscle2.set(65);
	c.genes.push_back(gc);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// mouth offs:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialMouthSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMouthAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// egglayer offs:
	ga.attribute = GENE_ATTRIB_EGG_EJECT_SPEED;
	ga.value.set(BodyConst::initialEggEjectSpeed);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// joint1 offs:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(8*BodyConst::initialJointSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(BodyConst::initialJointMaxPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_RESET_TORQUE;
	ga.value.set(BodyConst::initialJointResetTorque);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// bone1 offs:
	// grow bone2:
	gc.age = 7;
	gc.angle.set(0);
	gc.genomeOffset.set(20);
	gc.genomeOffsetJoint.set(8);
	gc.genomeOffsetMuscle1.set(14);
	gc.genomeOffsetMuscle2.set(14);
	c.genes.push_back(gc);

	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(PI/8);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_DENSITY;
	ga.value.set(BodyConst::initialBoneDensity);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// bone2 joint offs:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(BodyConst::initialJointMaxPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_RESET_TORQUE;
	ga.value.set(BodyConst::initialJointResetTorque);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// bone2 muscles offs:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMuscleAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// bone2 offs:
	// grow gripper:
	gc.age = 6;
	gc.part_type = GENE_PART_GRIPPER;
	gc.genomeOffset.set(14);
	gc.genomeOffsetJoint.set(8);
	gc.genomeOffsetMuscle1.set(18);
	gc.genomeOffsetMuscle2.set(18);
	c.genes.push_back(gc);

	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_DENSITY;
	ga.value.set(BodyConst::initialBoneDensity);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// gripper joint offs:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(BodyConst::initialJointMaxPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_RESET_TORQUE;
	ga.value.set(BodyConst::initialJointResetTorque);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// gripper offs:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// gripper muscle offs:

	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMuscleAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// bone1 muscles offs:
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(1.e-3f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMuscleAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	// finished with adding genes.
	// now we need to add some redundancy in between genes:
	int padding = 2;
	for (uint i=0; i<c.genes.size(); i+=padding+1) {
		for (int k=0; k<padding; k++)
			c.genes.insert(c.genes.begin()+i+1, GeneNoOp());
		if (c.genes[i].type == GENE_TYPE_SKIP) {
			c.genes[i].data.gene_skip.count.set(c.genes[i].data.gene_skip.count * (padding+1));
		}
		if (c.genes[i].type == GENE_TYPE_DEVELOPMENT) {
			c.genes[i].data.gene_command.genomeOffset.set(c.genes[i].data.gene_command.genomeOffset * (padding+1));
			c.genes[i].data.gene_command.genomeOffsetJoint.set(c.genes[i].data.gene_command.genomeOffsetJoint * (padding+1));
			c.genes[i].data.gene_command.genomeOffsetMuscle1.set(c.genes[i].data.gene_command.genomeOffsetMuscle1 * (padding+1));
			c.genes[i].data.gene_command.genomeOffsetMuscle2.set(c.genes[i].data.gene_command.genomeOffsetMuscle2 * (padding+1));
		}
	}

	return c;
}

Bug* Bug::newBasicBug(glm::vec2 position) {
	Genome g;
	g.first = g.second = createBasicChromosome(); // make a duplicate of all genes into the second chromosome
	return new Bug(g, 2*BodyConst::initialEggMass, position, glm::vec2(0));
}

Bug* Bug::newBasicMutantBug(glm::vec2 position) {
	Genome g;
	g.first = g.second = createBasicChromosome();
	GeneticOperations::alterChromosome(g.first);
	GeneticOperations::alterChromosome(g.second);
	GeneticOperations::fixGenesSynchro(g);
	return new Bug(g, 2*BodyConst::initialEggMass, position, glm::vec2(0));
}

glm::vec2 Bug::getPosition() {
	if (zygoteShell_)
		return vec3xy(zygoteShell_->getWorldTransformation());
	if (body_)
		return vec3xy(body_->getWorldTransformation());
	return glm::vec2(0);
}

glm::vec2 Bug::getVelocity() {
	if (zygoteShell_)
		return b2g(zygoteShell_->getBody().b2Body_->GetLinearVelocity());
	if (body_)
		b2g(body_->getBody().b2Body_->GetLinearVelocity());
	return glm::vec2(0);
}

void Bug::serialize(BinaryStream &stream) {
	if (!isAlive_)
		return;
	glm::vec2 pos = getPosition();
	stream << pos.x << pos.y;
	glm::vec2 vel = getVelocity();
	stream << vel.x << vel.y;
	float mass = zygoteShell_ ? zygoteShell_->getMass() : body_ ? body_->getMass_tree() : 0;
	assertDbg(mass > 0);
	stream << mass;
	stream << genome_;
}

void Bug::deserialize(BinaryStream &stream) {
	if (stream.getSize() == 0)
		return; // this was a dead bug
	float posx, posy, velx, vely, mass;
	stream >> posx >> posy >> velx >> vely >> mass;
	Genome genome;
	stream >> genome;
	std::unique_ptr<Bug> ptr(new Bug(genome, mass, glm::vec2(posx, posy), glm::vec2(velx, vely)));
	World::getInstance()->takeOwnershipOf(std::move(ptr));
}
