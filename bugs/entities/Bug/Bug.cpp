/*
 * Bug.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: bog
 */

#include "Bug.h"
#include "../../neuralnet/Network.h"
#include "../../neuralnet/functions.h"
#include "../../genetics/Gene.h"
#include "../../genetics/GeneDefinitions.h"
#include "../../genetics/constants.h"
#include "../../genetics/Ribosome.h"
#include "../../body-parts/ZygoteShell.h"
#include "../../body-parts/BodyConst.h"
#include "../../body-parts/EggLayer.h"
#include "../../body-parts/BodyPart.h"
#include "../../body-parts/FatCell.h"
#include "../../body-parts/JointPivot.h"
#include "../../serialization/GenomeSerialization.h"
#include "../Gamete.h"
#include "../WorldConst.h"

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

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

static const float DECODE_FREQUENCY = 5.f; // genes per second
static const float DECODE_PERIOD = 1.f / DECODE_FREQUENCY; // seconds
static const unsigned AABB_CACHE_MAX_FRAMES = 15;

std::atomic<unsigned> Bug::population {0};
std::atomic<unsigned> Bug::maxGeneration {0};
std::atomic<unsigned> Bug::freeZygotes {0};
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
	, cachedMassDirty_(true)
	, context_(*this, bodyPartsUpdateList_)
//	, initialFatMassRatio_(BodyConst::initialFatMassRatio)
	, minFatMasRatio_(BodyConst::initialMinFatMassRatio)
	, adultLeanMass_(BodyConst::initialAdultLeanMass)
	, growthSpeed_(BodyConst::initialGrowthSpeed)
	, reproductiveMassRatio_(BodyConst::initialReproductiveMassRatio)
	, developmentMassThreshRatio_(BodyConst::initialDevelopmentMassThreshRatio)
	, eggMass_(BodyConst::initialEggMass)
	, generation_(generation)
	, cachedAABBFramesOld_(randi(AABB_CACHE_MAX_FRAMES))
{
	LOGPREFIX("BUG");
	DEBUGLOGLN("new embryo [id="<<id<<"]; printing chromosomes:");
	DEBUGLOGLN("C1: " << genome.first.stringify());
	DEBUGLOGLN("C2: " << genome.second.stringify());

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
	mapBodyAttributes_[GENE_BODY_DEVELOPMENT_MASS_THRESH_RATIO] = &developmentMassThreshRatio_;

	if (generation_ > maxGeneration)
		maxGeneration = generation_;
	freeZygotes++;

#ifdef DEBUG
	debugValueGetters_["growthMassBuffer"] = [this] { return growthMassBuffer_; };
	debugValueGetters_["maxGrowthMassBuffer"] = [this] { return maxGrowthMassBuffer_; };
	debugValueGetters_["cachedLeanMass"] = [this] { return cachedLeanMass_; };
	debugValueGetters_["fatMass"] = [this] { return getTotalFatMass(); };
	debugValueGetters_["actualGrowthSpeed"] = [this] { return actualGrowthSpeed_.load(); };
	debugValueGetters_["eggGrowthSpeed"] = [this] { return eggGrowthSpeed_.load(); };
	debugValueGetters_["fatGrowthSpeed"] = [this] { return fatGrowthSpeed_.load(); };
	debugValueGetters_["frameFoodProcessed"] = [this] { return frameFoodProcessed_.load(); };
	debugValueGetters_["frameEnergyUsed"] = [this] { return frameEnergyUsed_.load(); };
#endif
}

Bug::~Bug() {
#ifdef DEBUG
	std::set<BodyPart*> deletedParts;
#endif
	for (unsigned i=0; i<bodyParts_.size(); i++) {
		auto &p = bodyParts_[i];
#ifdef DEBUG
		assertDbg(deletedParts.insert(p).second);
#endif
		p->destroy();
		p = nullptr;
	}
	bodyParts_.clear();
	for (auto &bp : deadBodyParts_) {
		if (bp != nullptr) {
#ifdef DEBUG
			assertDbg(deletedParts.insert(bp).second);
#endif
			bp->destroy();
			bp = nullptr;
		}
	}
	deadBodyParts_.clear();
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
			if (!isViable_) {
				// embryo not viable, discarded.
				DEBUGLOGLN("Embryo not viable. DISCARDED.");
				World::getInstance().queueDeferredAction([this] {
					zygoteShell_->die();
					deadBodyParts_.push_back(zygoteShell_);
					zygoteShell_ = nullptr;
					kill();
				});
				return;
			}

			population++; // new member of the bug population

			fixAllGeneValues();

			// compute fat amount and lean mass
			float fatMass = getTotalFatMass();
			float zygMass = zygoteShell_->getMass();
			cachedLeanMass_ = zygMass - fatMass;
			cachedMassDirty_ = false;

			for (auto e : eggLayers_) {
				e->setTargetEggMass(eggMass_);
			}

			// delete embryo shell
			World::getInstance().queueDeferredAction([this] {
				zygoteShell_->destroy();
				zygoteShell_ = nullptr;
			});

			delete ribosome_;
			ribosome_ = nullptr;

			for (auto bp : bodyParts_) {
				bp->onDied.add([this](BodyPart *dying) {
					if (isAlive_) {
						bodyParts_.erase(std::remove(bodyParts_.begin(), bodyParts_.end(), dying));
					}
					deadBodyParts_.push_back(dying);
					if (dying->getType() == BodyPartType::EGGLAYER) {
						// must remove from eggLayers_ vector
						eggLayers_.erase(std::remove(eggLayers_.begin(), eggLayers_.end(), dying));
					}
					// if not a joint, kill all neurons inside the body part
					// (for joints the synapses are broken when the joint breaks)
					if (dying->getType() != BodyPartType::JOINT_PIVOT && dying->getType() != BodyPartType::JOINT_WELD) {
						for (auto &n : bodyPartNeurons_[dying])
							killNeuron(n);
					}
					// no more bodyParts left alive?
					if (bodyParts_.size() == 0)
						kill();
				});
			}
#ifdef DEBUG
//			LOGLN("Body Structure begin ---------------------------------");
//			// print body structure
//			int i=0;
//			for (auto bp : bodyParts_) {
//				LOGLN("#" << i << " " << bp->getDebugName());
//				for (auto n : bp->neighbors()) {
//					int j = std::find(bodyParts_.begin(), bodyParts_.end(), n) - bodyParts_.begin();
//					LOGLN("\t" << n->getDebugName() << "\t -> #" << j);
//				}
//				i++;
//			}
//			LOGLN("Body Structure end -----------------------------------");
#endif
		}
	}
}

void Bug::fixAllGeneValues() {
//	initialFatMassRatio_.reset(clamp(initialFatMassRatio_.get(), 0.f, 1.f));
#warning "move constants into BodyConst"
	minFatMasRatio_.reset(clamp(minFatMasRatio_.get(), 0.f, 1.f));
	adultLeanMass_.reset(clamp(adultLeanMass_.get(), 0.f, 1.e+20f));
	growthSpeed_.reset(clamp(growthSpeed_.get(), 0.f, 1.e+20f));
	reproductiveMassRatio_.reset(clamp(reproductiveMassRatio_.get(), 0.f, 1.f));
	eggMass_.reset(clamp(eggMass_.get(), BodyConst::MinEggMass, BodyConst::MaxEggMass));
	maxGrowthMassBuffer_ = growthSpeed_ * 100;	// can hold enough growth mass for 100 seconds
	developmentMassThreshRatio_.reset(clamp(developmentMassThreshRatio_.get(), BodyConst::minDevelopmentMassThreshRatio, BodyConst::maxDevelopmentMassThreshRatio));
}

void Bug::updateDeadDecaying(float dt) {
	// body parts loose their nutrient value gradually until they are deleted
	for (unsigned i=0; i<deadBodyParts_.size(); i++) {
		if (!!!deadBodyParts_[i])
			continue;
		float decaySpeed = max((float)WorldConst::BodyDecaySpeedThresh, deadBodyParts_[i]->size() * WorldConst::BodyDecaySpeedDensity);
		deadBodyParts_[i]->consumeFoodValue(dt * decaySpeed);
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
			DEBUGLOGLN("bug DIED");
			--population; // one less bug
			isAlive_ = false;
			decltype(bodyParts_) parts;
			parts.swap(bodyParts_);	// avoid screwing iterator during onDied event handler
			for (auto b : parts)
				b->die();
		}
	});
}

void Bug::update(float dt) {
	PERF_MARKER_FUNC;
	if (isDeveloping_) {
		updateEmbryonicDevelopment(dt);
		return;
	}

#ifdef DEBUG
	eggGrowthSpeed_.store(0);
	fatGrowthSpeed_.store(0);
	frameFoodProcessed_.store(0);
	frameEnergyUsed_.store(0);
#endif

	cachedAABBFramesOld_++;

	{
		PERF_MARKER("update-bodyParts");
		bodyPartsUpdateList_.update(dt);
	}
	updateDeadDecaying(dt); // this updates all dead body parts

#ifdef DEBUG
	eggGrowthSpeed_.store(eggGrowthSpeed_ / dt);
	fatGrowthSpeed_.store(fatGrowthSpeed_ / dt);
#endif

	if (!isAlive_) {
		bool hasDeadParts = false;
		for (auto p : deadBodyParts_)
			if (p != nullptr) {
				hasDeadParts = true;
				break;
			}
		if (!hasDeadParts)
			destroy();
		return;
	}

	// take care of joints pending to be recreated
	if (!jointsToRecreate_.empty()) {
		decltype(jointsToRecreate_) jointsToRecreate;
		jointsToRecreate.swap(jointsToRecreate_);
		World::getInstance().queueDeferredAction([jointsToRecreate] {
			for (auto j : jointsToRecreate) {
				j->destroyFixtures();
			}
		});
		World::getInstance().queueDeferredAction([jointsToRecreate] {
			for (auto j : jointsToRecreate) {
				j->updateFixtures();
			}
		}, 1); // delay 1 frame to allow neighbouring parts to settle in place
	}

	{
		PERF_MARKER("update-neuralNet");
		lifeTime_ += dt;
		lifetimeOutput_.push_value(lifeTime_);
		neuralNet_->iterate(dt);
	}

	if (cachedMassDirty_) {
		// some part broke up, must recompute some things
		float oldLeanMass = cachedLeanMass_;
		cachedLeanMass_ = getLeanMass();
		adultLeanMass_.changeRel(cachedLeanMass_ / oldLeanMass);
		cachedMassDirty_ = false;
	}

	// residual energy consumption:
	consumeEnergy(cachedLeanMass_ * dt * BodyConst::ResidualEnergyConstant);

	// energy consumed by neurons:
	consumeEnergy(computeNeuralNetFrameEnergy(dt));

	// grow:
	if (cachedLeanMass_ < adultLeanMass_) {
		// juvenile, growing
		// max growth speed is dictated by genes
		float massToGrow = growthSpeed_ * dt;
		if (massToGrow > growthMassBuffer_)
			massToGrow = growthMassBuffer_;
		if (massToGrow > 0) {
			growthMassBuffer_ -= massToGrow;
			cachedLeanMass_ += massToGrow;
			for (auto b : bodyParts_)
				if (b->getType() != BodyPartType::FAT)
					b->applyScale(cachedLeanMass_ / (cachedLeanMass_ - massToGrow));
#ifdef DEBUG
			actualGrowthSpeed_.store(massToGrow / dt);
#endif
		}
	} else {
		// adult life
	}
}

float Bug::computeNeuralNetFrameEnergy(float dt) {
	float energy = 0;
	for (auto n : neuralNet_->neurons) {
		// the energy consumption rate per second for a neuron depends on its output value;
		// the higher the output value, the higher the consumption; the relationship is sub-linear
		energy += pow(abs(n->getValue()), 1/3.f) * BodyConst::NeuronEnergyRate * dt;
	}
	return energy;
}

void Bug::draw(Viewport* vp) {
	if (zygoteShell_) {
		zygoteShell_->draw(vp);
		ribosome_->drawCells(vp);
	} else {
		for (auto bp : bodyParts_)
			bp->draw(vp);
	}
	for (auto bp : deadBodyParts_)
		if (bp)
			bp->draw(vp);

	// draw debug data:
	if (/*ctx.enabledLayers.bugDebug*/ false) {
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


void Bug::onJointBreak(Joint* joint) {
	for (auto &s : jointSynapses_[joint])
		breakSynapse(s);
}

void Bug::killNeuron(Neuron* n) {
	n->disable();

}
void Bug::breakSynapse(std::pair<OutputSocket*, InputSocket*> &syn) {
	syn.first->removeTarget(syn.second);
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
	NOT_IMPLEMENTED;
}

void Bug::serialize(BinaryStream &stream) const {
	if (!isAlive_)
		return;
	// TODO
	NOT_IMPLEMENTED;
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
	if (stream.size() == 0)
		return; // this was a dead bug
	// TODO
	NOT_IMPLEMENTED;
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
	if (neuronIndex < 0 || (unsigned)neuronIndex >= neuralNet_->neurons.size())
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

float Bug::getTotalMass() const {
	// TODO optimize by caching
	float totalMass = 0;
	for (BodyPart* p : bodyParts_) {
		totalMass += p->mass();
	}
	return totalMass;
}

float Bug::getTotalFatMass() const {
	// TODO optimize by caching
	float totalFatMass = 0;
	for (BodyPart* p : bodyParts_) {
		if (p->getType() == BodyPartType::FAT)
			totalFatMass += p->mass();
	}
	return totalFatMass;
}

void Bug::consumeEnergy(float totalAmount) {
	// TODO optimize by accumulating all consumed energy during the frame and processing only once at the end (on update)
	float amountConsumed = 0;
	float prevValue = 1;
	/*
	 * this check (amountConsumed != prevValue) is required to prevent an infinite loop caused by float imprecision
	 * when remainingDebt gets very small (~1.e-10)
	 */
	while (amountConsumed < totalAmount - EPS && amountConsumed != prevValue) {
		prevValue = amountConsumed;
		// try to balance all fat cells by using energy from each one proportional to their size
		float remainingDebt = totalAmount - amountConsumed;
		float totalFatMass = getTotalFatMass();
		if (totalFatMass < EPS) {
			kill();
			return;
		}
		for (BodyPart* p : bodyParts_) {
			if (p->getType() != BodyPartType::FAT)
				continue;
			FatCell* f = static_cast<FatCell*>(p);
			float amountToConsume = f->mass() / totalFatMass * remainingDebt;
			amountConsumed += f->consumeEnergy(amountToConsume);
		}
	}
#ifdef DEBUG
	frameEnergyUsed_.store(frameEnergyUsed_ + totalAmount);
#endif
}

void Bug::onFoodProcessed(float mass) {
	/*
	 * if fat is below critical ratio, all food goes to replenish it
	 * else if not at full size, a fraction of food is used for filling up the growth buffer.
	 * if bigger than developmentMassThreshRatio_ * adultLeanMass_ some food is invested into making eggs
	 * the rest of the food turns into energy and fat.
	 */
	//LOGLN("PROCESS_FOOD "<<mass<<"======================");
	if (cachedMassDirty_)
		return;
#warning "return above loses some food mass"
	float fatMassRatio = getTotalFatMass() / getTotalMass();
	float growthMass = 0;
	float eggMass = 0;
	if (fatMassRatio >= minFatMasRatio_) {
		if (cachedLeanMass_ >= adultLeanMass_ * developmentMassThreshRatio_) {
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
		}
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
#ifdef DEBUG
	frameFoodProcessed_.store(frameFoodProcessed_ + mass);
	eggGrowthSpeed_.store(eggGrowthSpeed_ + eggMass);
#endif
	// use the left food to make fat and energy:
	// distribute the remaining mass to all fatCells, proportional to each one's size
	float massRemaining = mass - eggMass - growthMass;
	if (massRemaining < EPS)
		return;
	float totalFatMass = getTotalFatMass();
	for (BodyPart* p : bodyParts_) {
		if (p->getType() != BodyPartType::FAT)
			continue;
		FatCell* f = static_cast<FatCell*>(p);
		float amountToDistribute = f->mass() / totalFatMass * massRemaining;
		f->replenishFromMass(amountToDistribute);
	}
#ifdef DEBUG
	fatGrowthSpeed_.store(fatGrowthSpeed_ + massRemaining);
#endif
}

#ifdef DEBUG
float Bug::getDebugValue(std::string const name) {
	auto it = debugValueGetters_.find(name);
	if (it == debugValueGetters_.end())
		return NAN;
	return it->second();
}
#endif
