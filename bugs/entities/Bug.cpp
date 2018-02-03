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
#include "../body-parts/ZygoteShell.h"
#include "../body-parts/Torso.h"
#include "../body-parts/BodyConst.h"
#include "../body-parts/EggLayer.h"
#include "../body-parts/BodyPart.h"
#include "../serialization/GenomeSerialization.h"
#include "Bug/IMotor.h"
#include "Bug/ISensor.h"
#include "Gamete.h"

#include <boglfw/math/math3D.h>
#include <boglfw/math/aabb.h>
#include <boglfw/World.h>
#include <boglfw/serialization/BinaryStream.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/utils/log.h>
#include <boglfw/perf/marker.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <sstream>
#include <algorithm>
#include "../body-parts/JointPivot.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static const float DECODE_FREQUENCY = 5.f; // genes per second
static const float DECODE_PERIOD = 1.f / DECODE_FREQUENCY; // seconds
static const uint AABB_CACHE_MAX_FRAMES = 15;

std::atomic<uint> Bug::population {0};
std::atomic<uint> Bug::maxGeneration {0};
std::atomic<uint> Bug::freeZygotes {0};
std::atomic<uint64_t> Bug::nextId {1};

Bug::Bug(Genome const &genome, float zygoteMass, glm::vec2 position, glm::vec2 velocity, unsigned generation)
	: genome_(genome)
	, neuralNet_(new NeuralNet())
	, ribosome_(nullptr)
	, isAlive_(true)
	, isDeveloping_(true)
	, tRibosomeStep_(0)
//	, body_(nullptr)
	, zygoteShell_(nullptr)
	, growthMassBuffer_(0)
	, maxGrowthMassBuffer_(0)
	, cachedLeanMass_(0)
	, cachedMassDirty_(false)
	, context_(*this, bodyPartsUpdateList_)
//	, initialFatMassRatio_(BodyConst::initialFatMassRatio)
	, minFatMasRatio_(BodyConst::initialMinFatMassRatio)
	, adultLeanMass_(BodyConst::initialAdultLeanMass)
	, growthSpeed_(BodyConst::initialGrowthSpeed)
	, reproductiveMassRatio_(BodyConst::initialReproductiveMassRatio)
	, eggMass_(BodyConst::initialEggMass)
	, generation_(generation)
	, cachedAABBFramesOld_(randi(AABB_CACHE_MAX_FRAMES))
{
	LOGPREFIX("BUG");
	LOGLN("new embryo [id="<<id<<"]; printing chromosomes:");
	LOGLN("C1: " << genome.first.stringify());
	LOGLN("C2: " << genome.second.stringify());

	// create embryo shell:
	zygoteShell_ = new ZygoteShell(position, 0, velocity, 0, zygoteMass, context_);
	// zygote mass determines the overall bug size after decoding -> must have equal overal mass
	//zygoteShell_->setUpdateList(bodyPartsUpdateList_);

//	body_ = new Torso();
	//zygoteShell_->add(body_, 0);
//	body_->onFoodProcessed.add(std::bind(&Bug::onFoodProcessed, this, std::placeholders::_1));
//	body_->onMotorLinesDetached.add(std::bind(&Bug::onMotorLinesDetached, this, std::placeholders::_1));
//	body_->onBodyMassChanged.add([this] { cachedMassDirty_ = true; });
//	body_->setOwner(this);
	ribosome_ = new Ribosome(this);

//	mapBodyAttributes_[GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO] = &initialFatMassRatio_;
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
#warning "Destroy body parts here"
	/*else if (body_) {
		body_->destroy();
		body_ = nullptr;
	}*/ else {
		for (auto bp : deadBodyParts_)
			if (bp != nullptr)
				bp->destroy();
	}
	if (ribosome_)
		delete ribosome_;
}

void Bug::updateEmbryonicDevelopment(float dt) {
	PERF_MARKER_FUNC;
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
				World::getInstance().queueDeferredAction([this] {
					//zygoteShell_->die_tree();
//					body_->detach(false);
//					body_->destroy();
//					body_ = nullptr;
#warning "do whatever cleanup required here"
				});
				return;
#warning "this will live forever, must destroy bug instance and just leave a dummy zygote behind for food"
			}

			population++; // new member of the bug population

			float currentMass = 1.f;//body_->getMass_tree();
			float zygMass = zygoteShell_->getMass();

			fixAllGeneValues();

			// compute fat amount and scale up the torso to the correct size
//			float fatMass = zygMass * initialFatMassRatio_;
//			body_->setInitialFatMass(fatMass);
//			cachedLeanMass_ = zygMass - fatMass;

			zygoteShell_->updateCachedDynamicPropsFromBody();
			// commit all changes and create the physics bodys and fixtures:
			//body_->commit_tree(cachedLeanMass_/currentMass);

			// delete embryo shell
			World::getInstance().queueDeferredAction([this] {
//				body_->detach(false);
				zygoteShell_->destroy();
				zygoteShell_ = nullptr;
			});

			delete ribosome_;
			ribosome_ = nullptr;

			/*body_->applyRecursive([this](BodyPart* part) {
				part->onDied.add([this](BodyPart *dying) {
					World::getInstance().queueDeferredAction([this, dying] {
						dying->removeAllLinks();
					});
					deadBodyParts_.push_back(dying);
					if (dying->getType() == BodyPartType::EGGLAYER) {
						// must remove from eggLayers_ vector
						eggLayers_.erase(std::remove(eggLayers_.begin(), eggLayers_.end(), dying));
					}
				});
				return false;
			});*/
		}
	}
}

void Bug::fixAllGeneValues() {
//	initialFatMassRatio_.reset(clamp(initialFatMassRatio_.get(), 0.f, 1.f));
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
			World::getInstance().queueDeferredAction([this, i] {
				deadBodyParts_[i]->destroy();
				bodyPartsUpdateList_.remove(deadBodyParts_[i]);
				deadBodyParts_[i] = nullptr;
			});
		}
	}
}

void Bug::kill() {
	World::getInstance().queueDeferredAction([this] {
		if (isAlive_) {
			LOGLN("bug DIED");
			--population; // one less bug
			isAlive_ = false;
			//body_->die_tree();
//			body_ = nullptr;
		}
	});
}

void Bug::update(float dt) {
	PERF_MARKER_FUNC;
	if (isDeveloping_) {
		updateEmbryonicDevelopment(dt);
		return;
	}

	cachedAABBFramesOld_++;

	lifeTimeSensor_.update(dt);
	{
		PERF_MARKER("update-bodyParts");
		bodyPartsUpdateList_.update(dt);
	}
	updateDeadDecaying(dt); // this updates all dead body parts

	if (!isAlive_)
		return;

	{
		PERF_MARKER("update-neuralNet");
		neuralNet_->iterate();
	}

#warning "implement die!"	// TODO
//	if (body_->getFatMass() <= 0 && body_->getBufferedEnergy() <= 0) {
//		// we just depleted our energy supply and died
//		kill();
//		return;
//	}

	//LOGLN("leanMass: "<<body_->getMass_tree()<<"  eggMassBuf: "<<eggMassBuffer_<<";  fatMass: "<<body_->getFatMass()<<";  energy: "<<body_->getBufferedEnergy());

	if (cachedMassDirty_) {
		// some part broke up, must recompute some things
		float oldLeanMass = cachedLeanMass_;
//		body_->resetCachedMass();
#warning "getMass_tree() includes fat too after purging initialization data"
		cachedLeanMass_ = 1.f;//body_->getMass_tree();
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
		//body_->applyScale_tree(cachedLeanMass_ / body_->getMass_tree());
	} else {
		// adult life
	}
}

void Bug::draw(RenderContext const &ctx) {
	if (zygoteShell_) {
		zygoteShell_->draw(ctx);
		ribosome_->drawCells(ctx);
	} else
		for (auto bp : bodyParts_)
			bp->draw(ctx);
	for (auto bp : deadBodyParts_)
		if (bp)
			bp->draw(ctx);

	// draw debug data:
	if (ctx.enabledLayers.bugDebug) {
		auto x = [this](Viewport* vp) {
			glm::vec2 wldPos = getAABB().center();
			glm::vec3 viewpPos = vp->project({wldPos, 0.f});
			return viewpPos.x;
		};
		auto y = [this](Viewport* vp) {
			glm::vec2 wldPos = getAABB().center();
			glm::vec3 viewpPos = vp->project({wldPos, 0.f});
			return viewpPos.y;
		};

		std::stringstream ss;
		ss << id;
		GLText::get()->print(ss.str(), ViewportCoord{x, y}, 0.99f, 16, glm::vec3(0.2f, 1.f, 0.1f));
	}
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
	float fatMassRatio = 0; //TODO body_->getFatMass() / (cachedLeanMass_ + body_->getFatMass());
	throw std::runtime_error("Implement this!");
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
	// body_->replenishEnergyFromMass(mass - eggMass - growthMass);
	// TODO
	throw std::runtime_error("Implement this!");
}

void Bug::onMotorLinesDetached(std::vector<unsigned> const& lines) {
	if (!isAlive_ || !lines.size())
		return;
#ifdef DEBUG
	LOGPREFIX("Bug");
	LOG("motor lines detached: ");
#endif
	for (unsigned i : lines) {
		if (motorLines_[i].second) {
#ifdef DEBUG
			LOGNP(i << ", ");
#endif
			motorLines_[i].second->removeTarget(motorLines_[i].first);
		}
		motorLines_[i] = std::make_pair(nullptr, nullptr);
	}
#ifdef DEBUG
	LOGNP("\n");
#endif
}

Bug* Bug::newBasicBug(glm::vec2 position) {
	Genome g;
	g.first = g.second = createBasicChromosome(); // make a duplicate of all genes into the second chromosome
	return new Bug(g, 2*BodyConst::initialEggMass, position, glm::vec2(0), 1);
}

Bug* Bug::newBasicMutantBug(glm::vec2 position) {
	LOGPREFIX("newBasicMutantBug");
	Genome g;
	g.first = g.second = createBasicChromosome();
	GeneticOperations::alterChromosome(g.first);
	GeneticOperations::alterChromosome(g.second);
	GeneticOperations::fixGenesSynchro(g);
	return new Bug(g, 2*BodyConst::initialEggMass, position, glm::vec2(0), 1);
}

//glm::vec2 Bug::getVelocity() const {
//	if (zygoteShell_)
//		return b2g(zygoteShell_->getBody().b2Body_->GetLinearVelocity());
////	if (body_)
////		b2g(body_->getBody().b2Body_->GetLinearVelocity());
//	// TODO
//	throw std::runtime_error("Implement this!");
//	return glm::vec2(0);
//}

float Bug::getMass() const {
	return 1.f;//zygoteShell_ ? zygoteShell_->getMass() : body_ ? body_->getMass_tree() : 0;
	// TODO
	throw std::runtime_error("Implement this!");
}

void Bug::serialize(BinaryStream &stream) const {
	if (!isAlive_)
		return;
	// TODO
	throw std::runtime_error("Implement this!");
//	glm::vec2 pos = getPosition();
//	stream << pos.x << pos.y;
//	glm::vec2 vel = getVelocity();
//	stream << vel.x << vel.y;
	float mass = getMass();
	assertDbg(mass > 0);
	stream << mass;
	stream << generation_;
	stream << genome_;
}

void Bug::deserialize(BinaryStream &stream) {
	if (stream.getSize() == 0)
		return; // this was a dead bug
	// TODO
	throw std::runtime_error("Implement this!");
	float posx, posy, velx, vely, mass;
	stream >> /*posx >> posy >> velx >> vely >>*/ mass;
	unsigned generation;
	stream >> generation;
	Genome genome;
	stream >> genome;
	std::unique_ptr<Bug> ptr(new Bug(genome, mass, glm::vec2(posx, posy), glm::vec2(velx, vely), generation));
	World::getInstance().takeOwnershipOf(std::move(ptr));
}

float Bug::getNeuronValue(int neuronIndex) const {
	if (neuronIndex < 0 || neuronIndex >= neuralNet_->neurons.size())
		return 0;
	return neuralNet_->neurons[neuronIndex]->getValue();
}

// if precise, compute bbox from all body parts, no cache
// otherwise compute bbox from only bones and cache it, updating every N frames
aabb Bug::getAABB(bool requirePrecise) const {
	PERF_MARKER_FUNC;
	if (zygoteShell_)
		return zygoteShell_->getAABB();

	if (cachedAABB_.empty() || cachedAABBFramesOld_ < AABB_CACHE_MAX_FRAMES) {
		// update cached
		cachedAABB_ = aabb();
		for (auto bp : bodyParts_)
			if (bp->getType() == BodyPartType::BONE)	// only treat bones for a coarser but faster computation
				cachedAABB_ = cachedAABB_.reunion(bp->getAABB());
	}
	return cachedAABB_;
}
