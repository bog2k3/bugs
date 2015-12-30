/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "BodyConst.h"
#include "../math/box2glm.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include "../math/math2D.h"
#include "../utils/log.h"
#include "../utils/assert.h"
#include "../genetics/GeneDefinitions.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <cassert>
#include <sstream>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

BodyPartInitializationData::BodyPartInitializationData()
	: angleOffset(0)
	, lateralOffset(0)
	, size(BodyConst::initialBodyPartSize)
	, density(BodyConst::initialBodyPartDensity)
{
}

BodyPart::BodyPart(BodyPartType type, std::shared_ptr<BodyPartInitializationData> initialData)
	: type_(type)
	, parent_(nullptr)
	, children_{nullptr}
	, nChildren_(0)
	, committed_(false)
	, dontCreateBody_(false)
	, geneValuesCached_(false)
	, attachmentDirectionParent_(0)
	, angleOffset_(0)
	, lateralOffset_(0)
	, size_(0.01f)
	, density_(1.f)
	, initialData_(initialData)
	, updateList_(nullptr)
	, lastCommitSize_inv_(0)
	, destroyCalled_(false)
	, dead_(false)
{
	assertDbg (initialData != nullptr);

	registerAttribute(GENE_ATTRIB_LOCAL_ROTATION, initialData_->angleOffset);
	registerAttribute(GENE_ATTRIB_ATTACHMENT_OFFSET, initialData_->lateralOffset);
	registerAttribute(GENE_ATTRIB_SIZE, initialData_->size);
}

BodyPart::~BodyPart() {
	assertDbg(destroyCalled_);
}

void BodyPart::destroy() {
	destroyCalled_ = true;
	detach(true);

	for (int i=0; i<nChildren_; i++)
		children_[i]->destroy();
	delete this;
}

bool BodyPart::applyRecursive(std::function<bool(BodyPart* pCurrent)> pred) {
	if (pred(this))
		return true;
	for (int i=0; i<nChildren_; i++)
		if (children_[i]->applyRecursive(pred))
			return true;
	return false;
}

void BodyPart::addMotorLine(int lineId) {
	motorLines_.push_back(lineId);
	if (parent_)
		parent_->addMotorLine(lineId);
}

bool angleSpanOverlap(float angle1, float span1, float angle2, float span2, bool sweepPositive, float &outMargin) {
	// will set outMargin to negative if overlap, or positive shortest gap around element if no overlap
	// move angle1 in origin for convenience:
	angle2 = limitAngle(angle2 - angle1, PI);
	angle1 = 0;
	float a1p = span1*0.5f, a1n = -span1*0.5f;
	float a2p = angle2 + span2*0.5f, a2n = angle2 - span2*0.5f;
	if (a2p*a2n >= 0) {
		// both ends of span2 are on the same side of angle 1
		if (a2p > 0) { // both on the positive side
			outMargin = min(a2p, a2n) - a1p;
			if (!sweepPositive)
				outMargin = 2*PI-outMargin-span1-span2;
		} else {// both on the negative side
			outMargin = a1n - max(a2n, a2p);
			if (sweepPositive)
				outMargin = 2*PI-outMargin-span1-span2;
		}
	} else { // ends of span2 are on different sides of angle 1
		float d1 = a2n-a1p, d2 = a1n-a2p;
		outMargin = angle2 > 0 ? d1 : d2; // the smallest distance between the spans (positive) or the greatest overlap (negative)
		if (sweepPositive == (angle2<=0))
			outMargin = 2*PI-outMargin-span1-span2;
	}

	return outMargin < 0;
}

#ifdef DEBUG
void BodyPart::checkCircularBuffer(bool noOverlap, bool checkOrder) {
	constexpr float child_span = 1.9f*PI/MAX_CHILDREN; // 1.9f instead of 2 to allow some room for round-offs and shit
	float gapPos = 0, gapNeg = 0;
	for (int i=0; i<nChildren_; i++) {
		int in = circularNext(i, nChildren_);
		int ip = circularPrev(i, nChildren_);
		if (noOverlap)
			assertDbg(initialData_->circularBuffer[i].gapAfter >= 0 && initialData_->circularBuffer[i].gapBefore >= 0);
		assertDbg(children_[i]->attachmentDirectionParent_ >= 0 && children_[i]->attachmentDirectionParent_ < 2*PI+1.e-4f);
		assertDbg(initialData_->circularBuffer[i].gapAfter == initialData_->circularBuffer[in].gapBefore);
		assertDbg(initialData_->circularBuffer[i].gapBefore == initialData_->circularBuffer[ip].gapAfter);
		float pos = children_[i]->attachmentDirectionParent_;
		float posnext = children_[in]->attachmentDirectionParent_;
		assertDbg(nChildren_==1 || eqEps(limitAngle(pos + child_span + initialData_->circularBuffer[i].gapAfter - posnext, PI), 0, 1.e-4f));
		if (checkOrder) {
			if (noOverlap)
				assertDbg(i==nChildren_-1 || pos < posnext);
			else
				assertDbg(i==nChildren_-1 || pos <= posnext);
		}
		gapPos += initialData_->circularBuffer[i].gapAfter;
		gapNeg += initialData_->circularBuffer[i].gapBefore;
	}
	assertDbg(eqEps(gapPos, gapNeg, 1.e-4f));
	float sum = child_span*nChildren_+gapPos;
	assertDbg(nChildren_ == 0 || eqEps(sum, 2*PI, 1.e-4f));
}
#endif

void BodyPart::pushBodyParts(int index, float delta) {
	if (delta == 0)
		return;
	int di = sign(delta);
	int initialIndex = index;
#ifdef DEBUG
	checkCircularBuffer(false, false);
#endif
	while (true) {
		BodyPartInitializationData::angularEntry &entry = initialData_->circularBuffer[index];
		children_[index]->setAttachmentDirection(limitAngle(children_[index]->attachmentDirectionParent_ + delta, 2*PI));
		entry.gapBefore += delta;
		entry.gapAfter -= delta;
		int iprev = circularPrev(index, nChildren_);
		initialData_->circularBuffer[iprev].gapAfter = entry.gapBefore;
		int inext = circularNext(index, nChildren_);
		initialData_->circularBuffer[inext].gapBefore = entry.gapAfter;
		if (di > 0) {
			if (entry.gapAfter >= 0)
				break;
		} else /* (di < 0)*/ {
			// delta is negative here
			if (entry.gapBefore >= 0)
				break;
		}
		index = di > 0 ? circularNext(index, nChildren_) : circularPrev(index, nChildren_);
		assertDbg(index != initialIndex && "came around full circle; something's fucked up");
#ifdef DEBUG
		checkCircularBuffer(false, false);
#endif
	}
}

void BodyPart::addRef(BodyPart* part) {
	assertDbg(!!!initialData_); // this can only be called after initialization, otherwise it would screw the circular buffer
	children_[nChildren_++] = part;
}

float BodyPart::add(BodyPart* part, float angle) {
#ifdef DEBUG
	checkCircularBuffer(true, true);
#endif
	angle = limitAngle(angle, 2*PI);
	assertDbg(nChildren_ < (int)MAX_CHILDREN && initialData_);
	// determine the position in the circular buffer:
	int bufferPos = 0;
	while (bufferPos < nChildren_ && angle >= children_[bufferPos]->attachmentDirectionParent_)
		bufferPos++;
	for (int i=nChildren_; i>bufferPos; i--) {
		initialData_->circularBuffer[i] = initialData_->circularBuffer[i-1];
		children_[i] = children_[i-1];
	}
	nChildren_++;
	children_[bufferPos] = part;
	part->parent_ = this;
	part->setAttachmentDirection(angle);
	part->onAddedToParent();
	constexpr float span = 1.9f*PI/MAX_CHILDREN; // 1.9f instead of 2 to allow some room for round-offs and shit
	float gapLeftBefore = 2*PI-span, gapLeftAfter = 2*PI-span; // initial values valid for no other children case
	bool overlapsNext = false, overlapsPrev = false;
	if (nChildren_ > 1) {
		int nextPos = circularNext(bufferPos, nChildren_);
		float nextAngle = children_[nextPos]->attachmentDirectionParent_;
		constexpr float nextSpan = span;
		overlapsNext = angleSpanOverlap(angle, span, nextAngle, nextSpan, true, gapLeftAfter);
		int prevPos = circularPrev(bufferPos, nChildren_);
		float prevAngle = children_[prevPos]->attachmentDirectionParent_;
		constexpr float prevSpan = span;
		overlapsPrev = angleSpanOverlap(angle, span, prevAngle, prevSpan, false, gapLeftBefore);
		initialData_->circularBuffer[nextPos].gapBefore = gapLeftAfter;
		initialData_->circularBuffer[prevPos].gapAfter = gapLeftBefore;
	}
	assertDbg(gapLeftBefore + gapLeftAfter >= -span - 1.e-4f);
	initialData_->circularBuffer[bufferPos].set(gapLeftBefore, gapLeftAfter);
	// done
	if (overlapsNext || overlapsPrev) {
		fixOverlaps(bufferPos);
		// fix order if some parts crossed the zero line
		while (children_[0]->attachmentDirectionParent_ > children_[1]->attachmentDirectionParent_) {
			auto c0 = children_[0];
			auto b0 = initialData_->circularBuffer[0];
			for (int i=0; i<nChildren_-1; i++) {
				children_[i] = children_[i+1];
				initialData_->circularBuffer[i] = initialData_->circularBuffer[i+1];
			}
			children_[nChildren_-1] = c0;
			initialData_->circularBuffer[nChildren_-1] = b0;
		}
		while (children_[nChildren_-1]->attachmentDirectionParent_ < children_[nChildren_-2]->attachmentDirectionParent_) {
			auto cn1 = children_[nChildren_-1];
			auto bn1 = initialData_->circularBuffer[nChildren_-1];
			for (int i=nChildren_-1; i>0; i--) {
				children_[i] = children_[i-1];
				initialData_->circularBuffer[i] = initialData_->circularBuffer[i-1];
			}
			children_[0] = cn1;
			initialData_->circularBuffer[0] = bn1;
		}
	}
#ifdef DEBUG
	checkCircularBuffer(true, true);
#endif
	return children_[bufferPos]->attachmentDirectionParent_;
}

void BodyPart::fixOverlaps(int startIndex) {
#ifdef DEBUG
	checkCircularBuffer(false, true);
#endif
	bool overlapsNext = initialData_->circularBuffer[startIndex].gapAfter <= 0;
	bool overlapsPrev = initialData_->circularBuffer[startIndex].gapBefore <= 0;
	constexpr float span = 1.9f*PI/MAX_CHILDREN; // 1.9f instead of 2 to allow some room for round-offs and shit
	// more iterations may be required, since the first gaps found may not be enough:
	float gapNeeded = (overlapsNext ? -initialData_->circularBuffer[startIndex].gapAfter : 0) +
						(overlapsPrev ? -initialData_->circularBuffer[startIndex].gapBefore : 0);
	while (gapNeeded > 0) {
		assertDbg(gapNeeded <= span+1.e-4);
		// walk the circular buffer and compute 'mass' and gap
		float MBefore = 0, gapBefore = 0; // compute push 'mass' and available gap before angle
		float MAfter = 0, gapAfter = 0; // compute push 'mass' and available gap after angle
		int firstNext = circularNext(startIndex, nChildren_), firstPrev = circularPrev(startIndex, nChildren_);
		bool wrapAround = false;
		if (!overlapsNext) {
			MAfter = span;
			gapAfter = initialData_->circularBuffer[startIndex].gapAfter;
		} else {
			// walk positive:
			int iNext = startIndex;
			do {
				iNext = circularNext(iNext, nChildren_);
				MAfter += span;
				gapAfter = initialData_->circularBuffer[iNext].gapAfter;
			} while (gapAfter < EPS && (nChildren_ == 2 || iNext != firstPrev));
			if (nChildren_ > 2 && iNext == firstPrev) {	// forward wrap around
				gapAfter = 0;
				wrapAround = true;
			}
		}
		assertDbg(wrapAround || gapAfter > 0);
		if (!overlapsPrev) {
			MBefore = span;
			gapBefore = initialData_->circularBuffer[startIndex].gapBefore;
		} else {
			// walk negative:
			int iPrev = startIndex;
			do {
				iPrev = circularPrev(iPrev, nChildren_);
				MBefore += span;
				gapBefore = initialData_->circularBuffer[iPrev].gapBefore;
			} while (gapBefore < EPS && (nChildren_ == 2 || iPrev != firstNext));
			if (nChildren_ > 2 && iPrev == firstNext) {	// backward wrap around
				gapBefore = 0;
				wrapAround = true;
			}
		}
		assertDbg(wrapAround || gapBefore > 0);
		assertDbg(MAfter*MBefore != 0);
		float MRatio = MAfter / MBefore;
		float pushBef = gapNeeded / (1 + MRatio);
		float pushAft = pushBef * MRatio;
		assertDbg(eqEps(pushBef+pushAft, gapNeeded, 1.e-4f));
		// check if there is a single gap:
		if (nChildren_ == 2 || wrapAround) {
			// distribute the semi-gaps proportional to the mass ratio
			float gapSum = gapBefore + gapAfter;
			gapAfter = pushAft / gapNeeded * gapSum;
			gapBefore = gapSum - gapAfter;
		}
		// bool willOverlapNext = false, willOverlapPrev = false;
		if (pushBef > gapBefore) {
			pushAft *= gapBefore / pushBef;
			pushBef = gapBefore;
			// willOverlapPrev = true;
		}
		if (pushAft > gapAfter) {
			pushBef *= gapAfter / pushAft;
			pushAft = gapAfter;
			//willOverlapNext = true;
		}
		if (pushAft > 0)
			pushBodyParts(overlapsNext ? firstNext : startIndex, pushAft);	// must alter the attachmentDirectionParent of the siblings
		if (pushBef > 0)
			pushBodyParts(overlapsPrev ? firstPrev : startIndex, -pushBef); // -||-
		if (overlapsNext && overlapsPrev) {
			float prevPos = children_[startIndex]->attachmentDirectionParent_;
			children_[startIndex]->attachmentDirectionParent_ += (pushAft - pushBef) * 0.5f; // adjust angle to the middle of the gap
			initialData_->circularBuffer[startIndex].gapAfter += prevPos - children_[startIndex]->attachmentDirectionParent_;
			initialData_->circularBuffer[circularNext(startIndex, nChildren_)].gapBefore = initialData_->circularBuffer[startIndex].gapAfter;
			initialData_->circularBuffer[startIndex].gapBefore += children_[startIndex]->attachmentDirectionParent_ - prevPos;
			initialData_->circularBuffer[circularPrev(startIndex, nChildren_)].gapAfter = initialData_->circularBuffer[startIndex].gapBefore;
			// bring the angle back into [0, 2*PI) range:
			children_[startIndex]->attachmentDirectionParent_ = limitAngle(children_[startIndex]->attachmentDirectionParent_, 2*PI);
		}

		overlapsNext = initialData_->circularBuffer[startIndex].gapAfter <= 0;
		overlapsPrev = initialData_->circularBuffer[startIndex].gapBefore <= 0;
		gapNeeded = (overlapsNext ? -initialData_->circularBuffer[startIndex].gapAfter : 0) +
					(overlapsPrev ? -initialData_->circularBuffer[startIndex].gapBefore : 0);

#ifdef DEBUG
		checkCircularBuffer(false, false);
#endif
	}
	assertDbg(initialData_->circularBuffer[startIndex].gapAfter >= 0);
	assertDbg(initialData_->circularBuffer[startIndex].gapBefore >= 0);
#ifdef DEBUG
	checkCircularBuffer(true, false);
#endif
}

void BodyPart::detach(bool die) {
	if (parent_) {
		// first must detach all neural connections
		detachMotorLines(motorLines_);
		parent_->remove(this);
		onDetachedFromParent();
		parent_->hierarchyMassChanged();
	}
	parent_ = nullptr;
	if (die && !dead_)
		die_tree();
}

void BodyPart::detachMotorLines(std::vector<unsigned> const& lines) {
	if (parent_)
		parent_->detachMotorLines(lines);
}

void BodyPart::remove(BodyPart* part) {
	for (int i=0; i<nChildren_; i++)
		if (children_[i] == part) {
			children_[i] = children_[--nChildren_];
			break;
		}
}

glm::vec2 BodyPart::getUpstreamAttachmentPoint() {
	if (!parent_)
		return glm::vec2(0);
	else {
		glm::vec2 point(parent_->getChildAttachmentPoint(attachmentDirectionParent_));
		assertDbg(!std::isnan(point.x) && !std::isnan(point.y));
		return point;
	}
}

void BodyPart::commit_tree(float initialScale) {
	if (!committed_) {
		initialData_->size.changeRel(initialScale);
		cacheInitializationData();
		geneValuesCached_ = true;
		purge_initializationData();
		computeBodyPhysProps();
	} else
		reverseUpdateCachedProps();
	lastCommitSize_inv_ = 1.f / size_;
	// perform commit on local node:
	if (type_ != BodyPartType::JOINT) {
		if (!physBody_.b2Body_ && !dontCreateBody_)
			physBody_.create(cachedProps_);
		commit();
	}
	// perform recursive commit on all non-muscle children:
	for (int i=0; i<nChildren_; i++) {
		if (children_[i]->type_ != BodyPartType::MUSCLE)
			children_[i]->commit_tree(initialScale);
	}
	// muscles go after all other children:
	for (int i=0; i<nChildren_; i++) {
		if (children_[i]->type_ == BodyPartType::MUSCLE)
			children_[i]->commit_tree(initialScale);
	}

	if (type_ == BodyPartType::JOINT) {
		commit();
	}
	committed_ = true;
}

void BodyPart::purge_initializationData() {
	initialData_.reset();
}

glm::vec2 BodyPart::getParentSpacePosition() {
	if (!geneValuesCached_)
		cacheInitializationData();
	glm::vec2 upstreamAttach = getUpstreamAttachmentPoint();
	glm::vec2 localOffset = getChildAttachmentPoint(PI - angleOffset_);
	assertDbg(!std::isnan(localOffset.x) && !std::isnan(localOffset.y));
	float angle;
	angle = attachmentDirectionParent_ + angleOffset_;
	glm::vec2 ret(upstreamAttach - glm::rotate(localOffset, angle));
	assertDbg(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
#warning "must take into account lateral offset"
}

void BodyPart::reverseUpdateCachedProps() {
	// reverse the magic here: get values from the physics engine and put them in our cached props
	// in order to facilitate a recommit with changed data.
	glm::vec3 worldTransform = getWorldTransformation();
	cachedProps_.angle = worldTransform.z;
	cachedProps_.position = vec3xy(worldTransform);
	if (physBody_.b2Body_) {
		cachedProps_.velocity = b2g(physBody_.b2Body_->GetLinearVelocity());
		cachedProps_.angularVelocity = physBody_.b2Body_->GetAngularVelocity();
	} else if (parent_) {
		cachedProps_.velocity = parent_->cachedProps_.velocity;
		cachedProps_.angularVelocity = parent_->cachedProps_.angularVelocity;
	}
}

void BodyPart::computeBodyPhysProps() {
	// do the magic here and update cachedProps from other positioning fields
	// cachedProps must be in world space
	// parent's cachedProps are assumed to be updated and in world space at this time
	PhysicsProperties parentProps = parent_ ? parent_->cachedProps_ : PhysicsProperties();
	cachedProps_.velocity = parentProps.velocity;
	cachedProps_.angularVelocity = parentProps.angularVelocity;
	// compute parent space position:
	glm::vec2 pos = getParentSpacePosition();
	assertDbg(!std::isnan(pos.x) && !std::isnan(pos.y));
	// compute world space position:
	pos = parentProps.position + glm::rotate(pos, parentProps.angle);
	assertDbg(!std::isnan(pos.x) && !std::isnan(pos.y));
	cachedProps_.position = pos;
	// compute world space angle:
	cachedProps_.angle = parentProps.angle + attachmentDirectionParent_ + angleOffset_;
	assertDbg(!std::isnan(cachedProps_.angle));
}

glm::vec3 BodyPart::getWorldTransformation() {
	if (physBody_.b2Body_) {
		return glm::vec3(b2g(physBody_.b2Body_->GetPosition()), physBody_.b2Body_->GetAngle());
	} else {
		// if not committed yet, must compute these values on the fly
		glm::vec3 parentTransform(parent_ ? parent_->getWorldTransformation() : glm::vec3(0));
		glm::vec2 pos = getParentSpacePosition(); // this will temporarily cache gene values as well
		return parentTransform + glm::vec3(
				glm::rotate(pos, parentTransform.z),
				attachmentDirectionParent_ + angleOffset_);
	}
}

void BodyPart::draw(RenderContext const& ctx) {
	if (committed_)
		return;
	glm::vec3 trans = getWorldTransformation();
	glm::vec2 pos(trans.x, trans.y);
	ctx.shape->drawLine(pos + glm::vec2(-0.01f, 0), pos + glm::vec2(0.01f, 0), 0, glm::vec3(0.2f, 0.2f, 1.f));
	ctx.shape->drawLine(pos + glm::vec2(0, -0.01f), pos + glm::vec2(0, 0.01f), 0, glm::vec3(1.f, 0.2f, 0.2f));
}

void BodyPart::registerAttribute(gene_part_attribute_type type, CummulativeValue& value) {
	registerAttribute(type, 0, value);
}

void BodyPart::registerAttribute(gene_part_attribute_type type, unsigned index, CummulativeValue& value) {
	auto &attrVec = mapAttributes_[type];
	while (attrVec.size() < index + 1)
		attrVec.push_back(nullptr);
	attrVec[index] = &value;
}

UpdateList* BodyPart::getUpdateList() {
	if (updateList_)
		return updateList_;
	else if (parent_) {
		updateList_ = parent_->getUpdateList();
		return updateList_;
	} else
		return nullptr;
}

float BodyPart::getMass_tree() {
	float mass = size_ * density_;

	for (int i=0; i<nChildren_; i++)
		mass += children_[i]->getMass_tree();
	return mass;
}

void BodyPart::applyScale_tree(float scale) {
	applyScale_treeImpl(scale, false);
}

bool BodyPart::applyScale_treeImpl(float scale, bool parentChanged) {
	size_ *= scale;
	bool committed_now = false, should_commit_joint = false;
	if (committed_) {
		if (size_ * lastCommitSize_inv_ > BodyConst::SizeThresholdToCommit
				|| size_ * lastCommitSize_inv_ < BodyConst::SizeThresholdToCommit_inv)
		{
			lastCommitSize_inv_ = 1.f / size_;
			if (type_ != BodyPartType::JOINT) {
				commit();
				committed_now = true;
			} else
				should_commit_joint = true;
		}
	}
	bool child_changed = false;
	for (int i=0; i<nChildren_; i++) {
		child_changed |= children_[i]->applyScale_treeImpl(scale, committed_now);
	}
	if (type_ == BodyPartType::JOINT && committed_ && (should_commit_joint || parentChanged || child_changed)) {
		// must commit a joint whenever the threshold is reached, or parent or child has committed
		commit();
		committed_now = true;
	}

	return committed_now;
}

void BodyPart::consumeEnergy(float amount) {
	if (parent_)
		parent_->consumeEnergy(amount);
}

void BodyPart::die_tree() {
	if (!dead_) {
		die();
		dead_ = true;
		foodValueLeft_ = size_ * density_;
	}
	for (int i=0; i<nChildren_; i++)
		children_[i]->die_tree();
	onDied.trigger(this);
}

void BodyPart::consumeFoodValue(float amount) {
	if (dead_) {
		foodValueLeft_ -= amount;
	}
}

void BodyPart::removeAllLinks() {
	parent_ = nullptr;
	nChildren_ = 0;
}

void BodyPart::reattachChildren() {
	if (committed_) {
		for (int i=0; i<nChildren_; i++) {
			children_[i]->commit();
		}
	}
}

void BodyPart::draw_tree(RenderContext const& ctx) {
	draw(ctx);
	for (int i=0; i<nChildren_; i++)
		children_[i]->draw_tree(ctx);
}

void BodyPart::cacheInitializationData() {
	angleOffset_ = limitAngle(initialData_->angleOffset, 2*PI);
	lateralOffset_ = initialData_->lateralOffset;
	size_ = initialData_->size.clamp(BodyConst::MinBodyPartSize, 1.e10f);
	density_ = initialData_->density.clamp(BodyConst::MinBodyPartDensity, BodyConst::MaxBodyPartDensity);
}

void BodyPart::hierarchyMassChanged() {
	if (parent_)
		parent_->hierarchyMassChanged();
}

void BodyPart::buildDebugName(std::stringstream &out_stream) const {
#ifndef DEBUG
	assert(false); // don't call this on release builds because it's slow
#endif
	if (parent_) {
		parent_->buildDebugName(out_stream);
		out_stream << "::";
	}
	// compute attachment slice:
	int slice = -1;
	if (parent_)
		slice = (int)(getAttachmentAngle() / (2*PI) * MAX_CHILDREN);
	// compute own name from type & slice:
	switch (type_) {
	case BodyPartType::BONE:
		out_stream << "Bone";
		break;
	case BodyPartType::EGGLAYER:
		out_stream << "EggLayer";
		break;
	case BodyPartType::GRIPPER:
		out_stream << "Gripper";
		break;
	case BodyPartType::JOINT:
		out_stream << "Joint";
		break;
	case BodyPartType::MOUTH:
		out_stream << "Mouth";
		break;
	case BodyPartType::MUSCLE:
		out_stream << "Muscle";
		break;
	case BodyPartType::SENSOR_COMPASS:
		out_stream << "SensorCompass";
		break;
	case BodyPartType::SENSOR_DIRECTION:
		out_stream << "SensorDirection";
		break;
	case BodyPartType::SENSOR_PROXIMITY:
		out_stream << "SensorProximity";
		break;
	case BodyPartType::SENSOR_SIGHT:
		out_stream << "SensorSight";
		break;
	case BodyPartType::TORSO:
		out_stream << "Torso";
		break;
	case BodyPartType::ZYGOTE_SHELL:
		out_stream << "ZygoteShell";
		break;

	default:
		out_stream << "UNKNOWN";
		break;
	}
	if (slice >= 0) {
		out_stream << "(" << slice << ")";
	}
}

std::string BodyPart::getDebugName() const {
	std::stringstream ss;
	buildDebugName(ss);
	return ss.str();
}
