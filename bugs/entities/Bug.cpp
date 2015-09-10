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
#include "Bug/IMotor.h"
#include "Bug/ISensor.h"
#include "Gamete.h"

const float DECODE_FREQUENCY = 5.f; // genes per second
const float DECODE_PERIOD = 1.f / DECODE_FREQUENCY; // seconds

unsigned Bug::population = 0;
unsigned Bug::maxGeneration = 0;
unsigned Bug::freeZygotes = 0;

Bug::Bug(Genome const &genome, float zygoteMass, glm::vec2 position, glm::vec2 velocity, unsigned generation)
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
	, generation_(generation)
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
	body_->setOwner(this);
	ribosome_ = new Ribosome(this);

	mapBodyAttributes_[GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO] = &initialFatMassRatio_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO] = &minFatMasRatio_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_ADULT_LEAN_MASS] = &adultLeanMass_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_GROWTH_SPEED] = &growthSpeed_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_REPRODUCTIVE_MASS_RATIO] = &reproductiveMassRatio_;
	mapBodyAttributes_[GENE_BODY_ATTRIB_EGG_MASS] = &eggMass_;

	ribosome_->addDefaultSensor(&lifeTimeSensor_);

	if (generation_ > maxGeneration)
		maxGeneration = generation_;
	freeZygotes++;
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
		if (!isDeveloping_) {	// finished development
			freeZygotes--;
			if (!isAlive_) {
				// embryo not viable, discarded.
				LOGLN("Embryo not viable. DISCARDED.");
				zygoteShell_->die_tree();
				body_->detach(false);
				body_->destroy();
				body_ = nullptr;
				return;
			}

			population++; // new member of the bug population

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
	if (isAlive_) {
		LOGLN("bug DIED");
		population--; // one less bug
		isAlive_ = false;
		body_->die_tree();
		body_ = nullptr;
	}
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

void Bug::onMotorLinesDetached(std::vector<unsigned> const& lines) {
	if (!isAlive_)
		return;
	for (unsigned i : lines) {
		assertDbg(i < motorLines_.size());
		if (motorLines_[i].second)
			motorLines_[i].second->removeTarget(motorLines_[i].first);
		motorLines_[i] = std::make_pair(nullptr, nullptr);
	}
}

Chromosome Bug::createBasicChromosome() {
	Chromosome c;

	constexpr float body_size = 0.1f * 0.1f; // sq meters
	constexpr float body_init_fat_ratio = 0.5f;
	constexpr float body_min_fat_ratio = 0.1f;
	constexpr float body_adult_lean_mass = 4; // kg
	constexpr float muscle1_VMScoord = 5.f;
	constexpr float gripper_VMScoord = 10.f;
	constexpr float muscle2_VMScoord = 15.f;
	constexpr float musclePeriod = 3.f; // seconds
	constexpr float gripper_signal_threshold = -0.2f;

	// body size (sq meters)
	GeneAttribute ga;
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(body_size);
	c.genes.push_back(ga);

	//body attributes
	GeneBodyAttribute gba;
	gba.attribute = GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO;
	gba.value.set(body_init_fat_ratio);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO;
	gba.value.set(body_min_fat_ratio);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_ADULT_LEAN_MASS;
	gba.value.set(body_adult_lean_mass);
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

	// G+7

	// neural system

	// neuron #0 transfer:
	GeneTransferFunction gt;
	gt.targetNeuron.set(0);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);

	// neuron #1 transfer:
	gt.targetNeuron.set(1);
	gt.functionID.set((int)transferFuncNames::FN_CONSTANT);
	c.genes.push_back(gt);
	// neuron #1 constant:
	GeneNeuralConstant gnc;
	gnc.targetNeuron.set(1);
	gnc.value.set(PI);
	c.genes.push_back(gnc);

	// neuron #2 transfer:
	gt.targetNeuron.set(2);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #2 output VMS coord
	GeneNeuronOutputCoord goc;
	goc.srcNeuronVirtIndex.set(2);
	goc.outCoord.set(muscle1_VMScoord);
	c.genes.push_back(goc);

	// neuron #3 transfer:
	gt.targetNeuron.set(3);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #3 output VMS coord
	goc.srcNeuronVirtIndex.set(3);
	goc.outCoord.set(muscle2_VMScoord);
	c.genes.push_back(goc);

	// neuron #4 transfer:
	gt.targetNeuron.set(4);
	gt.functionID.set((int)transferFuncNames::FN_CONSTANT);
	c.genes.push_back(gt);
	// neuron #4 constant:
	gnc.targetNeuron.set(4);
	gnc.value.set(gripper_signal_threshold);
	c.genes.push_back(gnc);

	// neuron #5 transfer:
	gt.targetNeuron.set(5);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #5 output VMS coord
	goc.srcNeuronVirtIndex.set(5);
	goc.outCoord.set(gripper_VMScoord);
	c.genes.push_back(goc);

	// G+11


	GeneSynapse gs;

	// synapse 0 to 2
	gs.from.set(0);
	gs.to.set(1);
	gs.weight.set(2*PI / musclePeriod);
	c.genes.push_back(gs);

	// synapse 0 to 3
	gs.from.set(0);
	gs.to.set(3);
	gs.weight.set(2*PI / musclePeriod);
	c.genes.push_back(gs);

	// synapse 1 to 3
	gs.from.set(1);
	gs.to.set(3);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 3 to 5
	gs.from.set(3);
	gs.to.set(5);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 4 to 5
	gs.from.set(4);
	gs.to.set(5);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// G+5

	// grow Mouth:
	GeneProtein gp;
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_B);	// X+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	GeneOffset go;
	go.targetSegment.set(0);
	go.offset.set(50);
	c.genes.push_back(go);

	// G+5

	// grow Bone 1:
	gp.targetSegment.set(8);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_C);	// Y-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(8);
	go.offset.set(62);
	c.genes.push_back(go);

	// G+5

	// grow Muscle 1:
	gp.targetSegment.set(7);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(7);
	go.offset.set(136);
	c.genes.push_back(go);

	// G+5

	// grow Muscle 2:
	gp.targetSegment.set(9);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(9);
	go.offset.set(136);
	c.genes.push_back(go);

	// G+5

	// grow Egg Layer:
	gp.targetSegment.set(5);
	gp.protein.set(GENE_PROT_B);	// X+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(5);
	go.offset.set(56);
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// G+7

	// mouth offs (G 50):
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

	// G+6

	// egglayer offs (G 56):
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

	// G+6

	// bone1 offs (G 62):
	GeneJointOffset gjo;
	gjo.offset.set(23);
	c.genes.push_back(gjo);

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

	// G+6

	// grow Bone 2:
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_C);	// Y-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(0);
	go.offset.set(33);				// (95-62) offset is relative to the current part's
	c.genes.push_back(go);

	// G+5

	// grow Muscle 3:
	gp.targetSegment.set(1);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(1);
	go.offset.set(27);
	c.genes.push_back(go);

	// G+5

	// grow Muscle 4:
	gp.targetSegment.set(15);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(15);
	go.offset.set(27);
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// G+7

	// bone 1 joint offs (G 83):
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 3);
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


	// G+6

	// muscle 3 & 4 offs (G 89):
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

	// G+6

	// bone2 offset (G 95):
	gjo.offset.set(23);
	c.genes.push_back(gjo);

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

	// G+6

	// grow gripper:
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(0);
	go.offset.set(28);
	c.genes.push_back(go);

	// G+5

	// grow gripper muscle 1:
	gp.targetSegment.set(1);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(0);
	go.offset.set(35);
	c.genes.push_back(go);

	// G+5

	// grow gripper muscle 2:
	gp.targetSegment.set(15);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(15);
	go.offset.set(35);
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// G+7

	// bone 2 joint offs (G 118):
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

	// G+5

	// gripper offs (G 123):
	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(BodyConst::initialJointMaxPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_RESET_TORQUE;
	ga.value.set(BodyConst::initialJointResetTorque);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// G+7

	// gripper muscle offs (G 130):
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

	// G+6

	// bone1 muscles 1&2 offs (G 136):
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

	// G+4
	// G 140 total

	// finished with adding genes.
	// now we need to add some redundancy in between genes:
	int padding = 2;
	for (uint i=0; i<c.genes.size(); i+=padding+1) {
		for (int k=0; k<padding; k++)
			c.genes.insert(c.genes.begin()+i+1, GeneNoOp());
		if (c.genes[i].type == GENE_TYPE_SKIP) {
			c.genes[i].data.gene_skip.count.set(c.genes[i].data.gene_skip.count * (padding+1));
		}
		if (c.genes[i].type == GENE_TYPE_OFFSET) {
			c.genes[i].data.gene_offset.offset.set(c.genes[i].data.gene_offset.offset * (padding+1));
		}
	}

	return c;
}

Bug* Bug::newBasicBug(glm::vec2 position) {
	Genome g;
	g.first = g.second = createBasicChromosome(); // make a duplicate of all genes into the second chromosome
	return new Bug(g, 2*BodyConst::initialEggMass, position, glm::vec2(0), 1);
}

Bug* Bug::newBasicMutantBug(glm::vec2 position) {
	Genome g;
	g.first = g.second = createBasicChromosome();
	GeneticOperations::alterChromosome(g.first);
	GeneticOperations::alterChromosome(g.second);
	GeneticOperations::fixGenesSynchro(g);
	return new Bug(g, 2*BodyConst::initialEggMass, position, glm::vec2(0), 1);
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

float Bug::getMass() {
	return zygoteShell_ ? zygoteShell_->getMass() : body_ ? body_->getMass_tree() : 0;
}

void Bug::serialize(BinaryStream &stream) {
	if (!isAlive_)
		return;
	glm::vec2 pos = getPosition();
	stream << pos.x << pos.y;
	glm::vec2 vel = getVelocity();
	stream << vel.x << vel.y;
	float mass = getMass();
	assertDbg(mass > 0);
	stream << mass;
	stream << generation_;
	stream << genome_;
}

void Bug::deserialize(BinaryStream &stream) {
	if (stream.getSize() == 0)
		return; // this was a dead bug
	float posx, posy, velx, vely, mass;
	stream >> posx >> posy >> velx >> vely >> mass;
	unsigned generation;
	stream >> generation;
	Genome genome;
	stream >> genome;
	std::unique_ptr<Bug> ptr(new Bug(genome, mass, glm::vec2(posx, posy), glm::vec2(velx, vely), generation));
	World::getInstance()->takeOwnershipOf(std::move(ptr));
}
